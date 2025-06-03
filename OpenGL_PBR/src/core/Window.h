#pragma once

#include <string>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>


namespace core {

    class Window {
    public:
        // 构造：创建一个指定宽高和标题的 GLFW 窗口
        Window(int width, int height, const std::string& title);

        // 析构时销毁 GLFW 窗口（若尚未销毁）
        ~Window();

        // 获取底层 GLFWwindow* 句柄
        GLFWwindow* GetGLFWwindow() const;

        // 检查窗口是否收到关闭信号
        bool ShouldClose() const;

        // 轮询 GLFW 事件
        void PollEvents() const;

        // 交换前后缓冲区
        void SwapBuffers() const;

        // 手动销毁窗口（可多次调用，内部会判断是否已销毁）
        void Shutdown();

        // 注册帧缓冲区大小变化回调（常用来动态调用 glViewport）
        void SetFramebufferSizeCallback(GLFWframebuffersizefun callback);

    private:
        GLFWwindow* m_Window;    // 底层 GLFW 窗口指针
        int          m_Width;    // 窗口初始宽度
        int          m_Height;   // 窗口初始高度
        std::string  m_Title;    // 窗口标题
    };

} // namespace core
