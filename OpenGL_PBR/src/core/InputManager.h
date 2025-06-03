#pragma once

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include "Camera.h"

namespace core {

    class InputManager {
    public:
        // 构造时传入 GLFWwindow* 与要控制的 Camera*
        InputManager(GLFWwindow* window, Camera* camera);
        ~InputManager();

        // 每帧调用，处理键盘移动（W/A/S/D）与关闭窗口（ESC）
        // deltaTime 用于让不同帧率下移动速度保持一致
        void ProcessInput(float deltaTime);

    private:
        GLFWwindow* m_Window;   // 底层 GLFW 窗口指针
        Camera* m_Camera;  // 受控的相机指针

        float m_LastX;          // 上一次鼠标 X 坐标
        float m_LastY;          // 上一次鼠标 Y 坐标
        bool  m_FirstMouse;     // 是否首次捕获鼠标，用于初始化 m_LastX/m_LastY

        // GLFW 鼠标移动回调（静态函数），内部转发给实例方法
        static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
        // GLFW 滚轮回调（静态函数），内部转发给实例方法
        static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

        // 实例的方法：真正处理鼠标移动
        void OnMouseMove(double xpos, double ypos);
        // 实例的方法：真正处理滚轮变动
        void OnScroll(double yoffset);
    };

} // namespace core
