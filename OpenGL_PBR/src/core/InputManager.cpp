#include "InputManager.h"


namespace core {

    InputManager::InputManager(GLFWwindow* window, Camera* camera)
        : m_Window(window),
        m_Camera(camera),
        m_LastX(0.0f),
        m_LastY(0.0f),
        m_FirstMouse(true)
    {
        // 1. 把 this 存入 GLFWwindow 的 user pointer，供回调时取回
        glfwSetWindowUserPointer(m_Window, this);

        // 2. 注册 GLFW 回调
        glfwSetCursorPosCallback(m_Window, MouseCallback);
        glfwSetScrollCallback(m_Window, ScrollCallback);

        // 3. 隐藏并锁定光标到窗口中央
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // 4. 初始化 m_LastX/m_LastY 为窗口中心
        int width, height;
        glfwGetWindowSize(m_Window, &width, &height);
        m_LastX = static_cast<float>(width) / 2.0f;
        m_LastY = static_cast<float>(height) / 2.0f;
    }

    InputManager::~InputManager() {
        // 恢复光标为普通模式
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    // 每帧调用：处理键盘（W/A/S/D/ESC）输入
    void InputManager::ProcessInput(float deltaTime) {
        if (!m_Window || !m_Camera)
            return;

        // 1. 按 ESC 退出
        if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(m_Window, true);
        }
        // 2. WASD 控制相机移动
        if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS) {
            m_Camera->ProcessKeyboard(FORWARD, deltaTime);
        }
        if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS) {
            m_Camera->ProcessKeyboard(BACKWARD, deltaTime);
        }
        if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS) {
            m_Camera->ProcessKeyboard(LEFT, deltaTime);
        }
        if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS) {
            m_Camera->ProcessKeyboard(RIGHT, deltaTime);
        }
        // 如果需要上下移动，可以再加 Q/E 或空格/C 等按键
    }

    // 静态回调：鼠标移动时被 GLFW 调用
    void InputManager::MouseCallback(GLFWwindow* window, double xpos, double ypos) {
        // 取出对应的 InputManager 实例
        InputManager* mgr = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
        if (mgr) {
            mgr->OnMouseMove(xpos, ypos);
        }
    }

    // 静态回调：滚轮滚动时被 GLFW 调用
    void InputManager::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        InputManager* mgr = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
        if (mgr) {
            mgr->OnScroll(yoffset);
        }
    }

    // 实例方法：在鼠标移动时计算偏移并调用 Camera::ProcessMouseMovement
    void InputManager::OnMouseMove(double xpos, double ypos) {
        float x = static_cast<float>(xpos);
        float y = static_cast<float>(ypos);

        if (m_FirstMouse) {
            // 首次记录，避免大跳动
            m_LastX = x;
            m_LastY = y;
            m_FirstMouse = false;
            return;
        }

        float xoffset = x - m_LastX;
        float yoffset = m_LastY - y; // 这里要取反，因为屏幕 Y 向下为正

        m_LastX = x;
        m_LastY = y;

        // 将偏移传递给 Camera 更新 yaw 和 pitch
        if (m_Camera)
            m_Camera->ProcessMouseMovement(xoffset, yoffset);
    }

    // 实例方法：在滚轮滚动时调用 Camera::ProcessMouseScroll
    void InputManager::OnScroll(double yoffset) {
        if (m_Camera)
            m_Camera->ProcessMouseScroll(static_cast<float>(yoffset));
    }

} // namespace core
