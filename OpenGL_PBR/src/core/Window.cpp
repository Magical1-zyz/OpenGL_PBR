#include "Window.h"


namespace core {

    Window::Window(int width, int height, const std::string& title)
        : m_Window(nullptr),
        m_Width(width),
        m_Height(height),
        m_Title(title)
    {
        // （外部已调用 glfwInit()，这里只假设它已成功执行）
        // 创建 GLFW 窗口
        m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
        if (!m_Window) {
            std::cerr << "[Error] Failed to create GLFW window \"" << m_Title << "\"\n";
            glfwTerminate();
            std::exit(EXIT_FAILURE);
        }
        // 设置当前上下文
        glfwMakeContextCurrent(m_Window);
        // 默认开启垂直同步：交换缓冲时等待 VSync
        glfwSwapInterval(1);
    }

    Window::~Window() {
        // 确保在析构时销毁
        Shutdown();
    }

    GLFWwindow* Window::GetGLFWwindow() const {
        return m_Window;
    }

    bool Window::ShouldClose() const {
        if (!m_Window) return true;
        return glfwWindowShouldClose(m_Window);
    }

    void Window::PollEvents() const {
        // 轮询所有已经注册的回调（键盘、鼠标、窗口等事件）
        glfwPollEvents();
    }

    void Window::SwapBuffers() const {
        if (m_Window) {
            glfwSwapBuffers(m_Window);
        }
    }

    void Window::Shutdown() {
        if (m_Window) {
            // 销毁 GLFW 窗口
            glfwDestroyWindow(m_Window);
            m_Window = nullptr;
        }
    }

    void Window::SetFramebufferSizeCallback(GLFWframebuffersizefun callback) {
        if (m_Window) {
            glfwSetFramebufferSizeCallback(m_Window, callback);
        }
    }

} // namespace core
