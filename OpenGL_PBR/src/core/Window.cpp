#include "Window.h"


namespace core {

    Window::Window(int width, int height, const std::string& title)
        : m_Window(nullptr),
        m_Width(width),
        m_Height(height),
        m_Title(title)
    {
        // 1. 初始化 GLFW
        if (!glfwInit()) {
            std::cerr << "[Error] Failed to initialize GLFW\n";
            std::exit(EXIT_FAILURE);
        }

        // 2. 在 GLFW 初始化之后立即注册错误回调（这样不会报“library not initialized”）
        glfwSetErrorCallback([](int error, const char* description) {
            std::cerr << "GLFW Error [" << error << "]: " << description << std::endl;
            });

        // 3. 设置 OpenGL 版本和 Profile
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        // 4. 创建 GLFW 窗口
        m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
        if (!m_Window) {
            std::cerr << "[Error] Failed to create GLFW window \"" << m_Title << "\"\n";
            glfwTerminate();
            std::exit(EXIT_FAILURE);
        }

        // 5. 设置当前上下文
        glfwMakeContextCurrent(m_Window);

        // 6. 载入 OpenGL 函数指针（glad）
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "[Error] Failed to initialize GLAD\n";
            glfwDestroyWindow(m_Window);
            glfwTerminate();
            std::exit(EXIT_FAILURE);
        }

        // 7. 设置视口
        glViewport(0, 0, m_Width, m_Height);

        // 8. 开启深度测试
        glEnable(GL_DEPTH_TEST);

        // 9. 垂直同步
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
