#include "Application.h"


Application::Application(int width, int height, const std::string& title)
    : m_ScreenWidth(width),
    m_ScreenHeight(height),
    m_WindowTitle(title)
{
    InitWindow();
    InitSystems();
}

Application::~Application() {
    Cleanup();
}

void Application::InitWindow() {
    if (!glfwInit()) {
        std::cerr << "[Error] Failed to initialize GLFW\n";
        std::exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_Window = std::make_unique<core::Window>(m_ScreenWidth, m_ScreenHeight, m_WindowTitle);
    GLFWwindow* raw = m_Window->GetGLFWwindow();
    if (!raw) {
        std::cerr << "[Error] Failed to create GLFW window\n";
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }
    glfwSetFramebufferSizeCallback(raw,
        [](GLFWwindow* w, int ww, int hh) {
            glViewport(0, 0, ww, hh);
            // 如果需要，还可通知 PBRRenderer 调整尺寸
            Application* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
            if (app) app->m_PBRRenderer->Resize(ww, hh);
        }
    );
    glfwSetCursorPosCallback(raw, [](GLFWwindow* w, double x, double y) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
        if (app->m_Camera)
            app->m_Camera->ProcessMouseMovement(
                static_cast<float>(x) - app->m_LastFrameTime, // 这里只示意，用实际差值
                app->m_LastFrameTime - static_cast<float>(y)  // 来计算偏移
            );
        });
    glfwSetScrollCallback(raw, [](GLFWwindow* w, double x, double y) {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(w));
        if (app->m_Camera)
            app->m_Camera->ProcessMouseScroll(static_cast<float>(y));
        });

    // 隐藏并锁定鼠标
    glfwSetInputMode(raw, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 关联 user pointer
    glfwSetWindowUserPointer(raw, this);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "[Error] Failed to initialize GLAD\n";
        std::exit(EXIT_FAILURE);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void Application::InitSystems() {

    // 1. 摄像机
    m_Camera = std::make_unique<core::Camera>(
        glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        -90.0f,
        0.0f
    );

    // 2. 输入管理
    m_InputManager = std::make_unique<core::InputManager>(m_Window->GetGLFWwindow(), m_Camera.get());

    // 3. PBRRenderer
    m_PBRRenderer = std::make_unique<renderer::PBRRenderer>(m_ScreenWidth, m_ScreenHeight);
    m_PBRRenderer->InitPBR("assets/textures/hdr/newport_loft.hdr");
}

void Application::Run() {
    while (!m_Window->ShouldClose()) {
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - m_LastFrameTime;
        m_LastFrameTime = currentFrame;

        m_Window->PollEvents();
        m_InputManager->ProcessInput(deltaTime);

        // 更新：如果有动画／物理逻辑
        Update(deltaTime);

        // 渲染
        Render();

        m_Window->SwapBuffers();
    }
}

void Application::Update(float deltaTime) {
    // 这里可以更新动画、物理。对于 PBR 示例而言，暂时不需要额外逻辑
}

void Application::Render() {
    // 直接调用 PBRRenderer 渲染一帧
    m_PBRRenderer->RenderPBRScene(*m_Camera);
}

void Application::Cleanup() {
    // PBRRenderer 析构会释放 GL 资源
    m_PBRRenderer.reset();
    m_Camera.reset();
    m_InputManager.reset();

    if (m_Window) {
        m_Window->Shutdown();
        m_Window.reset();
    }
    glfwTerminate();
}
