#include "InputManager.h"


namespace core {

    InputManager::InputManager(GLFWwindow* window, Camera* camera)
        : m_Window(window),
        m_Camera(camera),
        m_LastX(0.0f),
        m_LastY(0.0f),
        m_FirstMouse(true),
		m_RButtonPressedLastFrame(false)
    {
        // 1) 注册 this 到 GLFWwindow，方便静态回调中使用
        glfwSetWindowUserPointer(m_Window, this);

        // 2) 安装回调，但不要覆盖 ImGui 的回调，要链式调用
        //    a. 鼠标移动回调
        glfwSetCursorPosCallback(m_Window, MouseMoveCallback);

        //    b. 鼠标按钮回调
        glfwSetMouseButtonCallback(m_Window, MouseButtonCallback);

        //    c. 滚轮回调
        glfwSetScrollCallback(m_Window, ScrollCallback);

        // 3) 一开始让光标可见
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    InputManager::~InputManager() {
        // 析构时，把光标恢复成可见
        if (m_Window) {
            glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
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

    // GLFW 鼠标移动回调（链式调用 ImGui 回调 + 自己的逻辑）
    void InputManager::MouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
        // 先把事件转发给 ImGui
        ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

        // 再执行自己的摄像机移动逻辑
        InputManager* mgr = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
        if (mgr) {
            mgr->OnMouseMove(xpos, ypos);
        }
    }

    // GLFW 鼠标按钮回调（链式调用 ImGui 回调 + 自己的逻辑）
    void InputManager::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        // 先转发给 ImGui，让它更新 io.MouseDown[] / io.WantCaptureMouse
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

        // 再执行自己的逻辑，比如“右键按下/松开”切换光标
        InputManager* mgr = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
        if (mgr) {
            mgr->OnMouseButton(button, action);
        }
    }

    // GLFW 滚轮回调（链式调用 ImGui 回调 + 自己的缩放逻辑）
    void InputManager::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
        InputManager* mgr = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
        if (mgr) {
            mgr->OnScroll(yoffset);
        }
    }

    // 实例方法：在鼠标移动时计算偏移并调用 Camera::ProcessMouseMovement
    void InputManager::OnMouseMove(double xpos, double ypos) {
        // 仅在“鼠标右键按下”时，让摄像机响应
        if (!(glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)) {
            return;
        }

        float x = static_cast<float>(xpos);
        float y = static_cast<float>(ypos);

        if (m_FirstMouse) {
            // 首次进入旋转模式，先初始化上一次坐标，避免巨大跳跃
            m_LastX = x;
            m_LastY = y;
            m_FirstMouse = false;
            return;
        }

        float xoffset = x - m_LastX;
        float yoffset = m_LastY - y; // 注意：屏幕 Y 向下为正

        m_LastX = x;
        m_LastY = y;

        // 将偏移传递给 Camera 更新 yaw 和 pitch
        if (m_Camera) {
            m_Camera->ProcessMouseMovement(xoffset, yoffset);
        }
    }

    void InputManager::OnMouseButton(int button, int action) {
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (action == GLFW_PRESS && !m_RButtonPressedLastFrame) {
                // 右键刚刚按下 → 隐藏并锁定光标，进入旋转模式
                glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                m_FirstMouse = true;  // 重置，防止第一步跳跃
                m_RButtonPressedLastFrame = true;
            }
            else if (action == GLFW_RELEASE && m_RButtonPressedLastFrame) {
                // 右键刚刚松开 → 恢复光标可见
                glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                m_RButtonPressedLastFrame = false;
                // 下次右键再按下时会重新初始化 m_FirstMouse
            }
        }
    }

    // 实例方法：在滚轮滚动时调用 Camera::ProcessMouseScroll
    void InputManager::OnScroll(double yoffset) {
        if (m_Camera)
            m_Camera->ProcessMouseScroll(static_cast<float>(yoffset));
    }

} // namespace core
