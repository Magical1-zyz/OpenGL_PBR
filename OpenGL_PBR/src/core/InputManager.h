#pragma once

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include "Camera.h"
#include "imgui/backends/imgui_impl_glfw.h"

namespace core {

    class InputManager {
    public:
        // 构造时传入 GLFWwindow* 与要控制的 Camera*
        InputManager(GLFWwindow* window, Camera* camera);
        ~InputManager();

        // 每帧调用，处理键盘移动（W/A/S/D）与关闭窗口（ESC）
        // deltaTime 用于让不同帧率下移动速度保持一致
        void ProcessInput(float deltaTime);

        // 鼠标移动回调（GLFW 调用）
        static void MouseMoveCallback(GLFWwindow* window, double xpos, double ypos);

        // 鼠标按钮 回调
        static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

        // 滚轮滚动回调（GLFW 调用）
        static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    private:
        // 只有在鼠标右键按下时才对摄像机进行旋转
        void OnMouseMove(double xpos, double ypos);

        // 只有鼠标右键按下/松开时，切换光标模式
        void OnMouseButton(int button, int action);

        // 始终响应滚轮缩放
		void OnScroll(double yoffset);

        GLFWwindow* m_Window;   // 底层 GLFW 窗口指针
        Camera* m_Camera;  // 受控的相机指针

        // 记录上一次鼠标位置，用于计算偏移
        float  m_LastX;
        float  m_LastY;
        bool   m_FirstMouse;

        // 记录上一次鼠标右键状态
        bool   m_RButtonPressedLastFrame;
    };

} // namespace core
