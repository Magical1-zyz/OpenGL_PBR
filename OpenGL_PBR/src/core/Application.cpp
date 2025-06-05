#include "Application.h"

void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}


Application::Application(int width, int height, const std::string& title)
    : m_ScreenWidth(width),
    m_ScreenHeight(height),
    m_WindowTitle(title)
{
    // 1) 初始化窗口 + OpenGL 上下文（此时 GLFW 已经 init 并设置了 error callback）
    InitWindow();

    // 2) 初始化 ImGui
    InitImGui();

    // 3) 初始化摄像机、输入管理、PBR 渲染器
    m_Camera = std::make_unique<core::Camera>(glm::vec3(0.0f, 0.0f, 3.0f));
    m_InputManager = std::make_unique<core::InputManager>(m_Window->GetGLFWwindow(), m_Camera.get());
    m_PBRRenderer = std::make_unique<renderer::PBRRenderer>(m_ScreenWidth, m_ScreenHeight);

    // 4) 加载默认 HDRI、初始化 PBR 所需资源
    m_PBRRenderer->InitPBR("assets/textures/hdr/newport_loft.hdr");

    // 5) 初始化默认光源
    if (m_PBRRenderer->lightPositions.empty()) {
        m_PBRRenderer->lightPositions = {
            { -10.0f,  10.0f, 10.0f },
            {  10.0f,  10.0f, 10.0f },
            { -10.0f, -10.0f, 10.0f },
            {  10.0f, -10.0f, 10.0f }
        };
        m_PBRRenderer->lightColors = {
            {300.0f, 300.0f, 300.0f},
            {300.0f, 300.0f, 300.0f},
            {300.0f, 300.0f, 300.0f},
            {300.0f, 300.0f, 300.0f}
        };
    }
}

Application::~Application() {
    CleanupImGui();
	glfwTerminate();
}

void Application::InitWindow() {
    m_Window = std::make_unique<core::Window>(m_ScreenWidth, m_ScreenHeight, m_WindowTitle);

    // 注册帧缓冲大小回调
    m_Window->SetFramebufferSizeCallback(FramebufferSizeCallback);
}

void Application::InitImGui() {
    // 1) 创建 ImGui 上下文
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    // 2) 配置 ImGui 样式
    ImGui::StyleColorsDark();

    // 3) 绑定 GLFW + OpenGL3（注意：要先 make context current，再调用这些）
    ImGui_ImplGlfw_InitForOpenGL(m_Window->GetGLFWwindow(), true);
    ImGui_ImplOpenGL3_Init(m_GLVersion);
}

void Application::CleanupImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

//void Application::InitSystems() {
//
    //// 1. 摄像机
    //m_Camera = std::make_unique<core::Camera>(
    //    glm::vec3(0.0f, 0.0f, 3.0f),
    //    glm::vec3(0.0f, 1.0f, 0.0f),
    //    -90.0f,
    //    0.0f
    //);
//
//    // 2. 输入管理
//    m_InputManager = std::make_unique<core::InputManager>(m_Window->GetGLFWwindow(), m_Camera.get());
//
//    // 3. PBRRenderer
//    m_PBRRenderer = std::make_unique<renderer::PBRRenderer>(m_ScreenWidth, m_ScreenHeight);
//    m_PBRRenderer->InitPBR("assets/textures/hdr/newport_loft.hdr");
//}

void Application::Run() {
    while (!m_Window->ShouldClose()) {
        // 1) 计算 deltaTime
        float currentTime = static_cast<float>(glfwGetTime());
        float dt = currentTime - m_LastFrameTime;
        m_LastFrameTime = currentTime;

        m_FPS = 1.0f / dt;
        m_FrameTimeMs = dt * 1000.0f;

        // 2) 先处理键盘 + “右键按下/松开”状态，让 InputManager 更新自身
        m_InputManager->ProcessInput(dt);

        // 3) 启动 ImGui 一帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 4) 获取 ImGui IO，看它是否要捕获鼠标
        ImGuiIO& io = ImGui::GetIO();
        bool wantMouse = io.WantCaptureMouse;

        // 5) 决定光标模式：
        //    如果 ImGui 要捕获鼠标，一定显示光标（GLFW_CURSOR_NORMAL），
        //    否则就让 InputManager 里“是否按住右键”的逻辑来决定：
        if (wantMouse) {
            glfwSetInputMode(m_Window->GetGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else {
            // 如果“右键已经按下”，InputManager 里已经做了 glfwSetInputMode(..., DISABLED)，
            // 所以这里不再一律禁止。只有当“右键没按下”时，再把光标显示出来：
            if (glfwGetMouseButton(m_Window->GetGLFWwindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
                // 右键松开，光标可见
                glfwSetInputMode(m_Window->GetGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            else {
                // 右键按下时，InputManager 已经叫我们隐藏了光标，这里什么也不干
            }
        }

        // 6) 渲染 3D 场景
        Render();

        // 7) ImGui 界面
        /*ShowFrameStats();
        ShowSettings();*/
		ShowControls();

        // 8) ImGui 绘制到屏幕
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // 9) 交换缓冲、轮询事件
        m_Window->SwapBuffers();
        glfwPollEvents();
    }
}

void Application::Render() {
    // 1. 清理颜色和深度缓冲
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2. 渲染 PBR 场景（或你自己的 3D 渲染逻辑）
    m_PBRRenderer->RenderPBRScene(*m_Camera);
}


void Application::ShowFrameStats() {
    // 你可以给窗口加 ImGuiWindowFlags_Resizable，以保证它可以手动调整大小
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize;
    // 如果你想要手动拖动调整大小，可以去掉 ImGuiWindowFlags_AlwaysAutoResize
    // ImGuiWindowFlags window_flags = 0;

    ImGui::Begin("Stats", nullptr, window_flags);
    ImGui::Text("FPS: %.1f", m_FPS);
    ImGui::Text("Frame Time: %.2f ms", m_FrameTimeMs);
    ImGui::End();
}

void Application::ShowSettings() {
    // 先开启一个可调整大小的窗口
    ImGui::Begin("Lighting Settings");

    // 1) 显示并可修改 exposure / gamma（如果 PBRRenderer 暴露了的话）
    float expo = m_PBRRenderer->exposure;
    if (ImGui::SliderFloat("Exposure", &expo, 0.01f, 10.0f)) {
        m_PBRRenderer->exposure = expo;
    }
    float g = m_PBRRenderer->gamma;
    if (ImGui::SliderFloat("Gamma", &g, 0.1f, 3.0f)) {
        m_PBRRenderer->gamma = g;
    }

    // 2) 遍历所有光源，让用户修改位置和颜色
    for (int i = 0; i < (int)m_PBRRenderer->lightPositions.size(); ++i) {
        std::string prefix = "Light " + std::to_string(i) + ":  ";

        // 位置
        {
            glm::vec3& pos = m_PBRRenderer->lightPositions[i];
            std::string label = prefix + "Position";
            if (ImGui::DragFloat3(label.c_str(), &pos.x, 0.1f, -20.0f, 20.0f)) {
                // 只要用户拖动，就会修改 pos.x/y/z，PBRRenderer::RenderPBRScene() 会自动使用新值
            }
        }

        // 颜色（HDR 值，可以用 ColorEdit3）
        {
            glm::vec3& col = m_PBRRenderer->lightColors[i];
            std::string label = prefix + "Color";
            // ImGui 的 ColorEdit3 默认把值 clamp 到 [0,1]，如果你要 >1 的 HDR 亮度，可以先除 300 再乘回来，或者用 DragFloat3
            float tmp[3] = { col.r / 300.0f, col.g / 300.0f, col.b / 300.0f };
            if (ImGui::ColorEdit3(label.c_str(), tmp)) {
                col.r = tmp[0] * 300.0f;
                col.g = tmp[1] * 300.0f;
                col.b = tmp[2] * 300.0f;
            }
        }

        ImGui::Separator();
    }

    ImGui::End();

    // ===================== 相机设置 =====================
    ImGui::Begin("Camera Settings");
    {
        glm::vec3 camPos = m_Camera->Position;
        if (ImGui::DragFloat3("Camera Position", &camPos.x, 0.1f, -20.0f, 20.0f)) {
            // 用户修改后，更新摄像机位置
            m_Camera->Position = camPos;
        }
        // 如果你想让用户以 Euler 角度控制摄像机 yaw/pitch，也可以在 Camera 类加相应成员，并在这里用 DragFloat 调整
    }
    ImGui::End();
}

// 将 FPS/FrameTime + Lighting 设置 + Camera 设置，全部放到同一个 ImGui 窗口里
void Application::ShowControls() {
    // 可以在第一次打开时指定一个初始大小
    ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);

    // 不使用 AlwaysAutoResize，让用户能自由调整大小
    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_None);

    // —— 1) Performance 区（折叠条） —— 
    if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen)) {
        // “默认展开”（ImGuiTreeNodeFlags_DefaultOpen）可以去掉，改成默认收起
        ImGui::Text("FPS: %.1f", m_FPS);
        ImGui::Text("Frame Time: %.2f ms", m_FrameTimeMs);
        ImGui::Spacing();
    }

    // —— 2) Lighting Settings 区（折叠条） —— 
    if (ImGui::CollapsingHeader("Lighting Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        // 调整 PBRRenderer 的 exposure / gamma
        {
            float expo = m_PBRRenderer->exposure;
            if (ImGui::SliderFloat("Exposure", &expo, 0.01f, 10.0f)) {
                m_PBRRenderer->exposure = expo;
            }
        }
        {
            float g = m_PBRRenderer->gamma;
            if (ImGui::SliderFloat("Gamma", &g, 0.1f, 3.0f)) {
                m_PBRRenderer->gamma = g;
            }
        }

        ImGui::Separator();

        // 遍历光源，让用户修改位置和颜色
        for (int i = 0; i < (int)m_PBRRenderer->lightPositions.size(); ++i) {
            std::string prefix = "Light " + std::to_string(i) + ":  ";
            // 位置
            {
                glm::vec3& pos = m_PBRRenderer->lightPositions[i];
                std::string label = prefix + "Position";
                ImGui::DragFloat3(label.c_str(), &pos.x, 0.1f, -20.0f, 20.0f);
            }
            // 颜色：用 ColorEdit3，但要让颜色可以超过 [0,1]，可以先除 300，再乘回来
            {
                glm::vec3& col = m_PBRRenderer->lightColors[i];
                std::string label = prefix + "Color";
                float tmp[3] = { col.r / 300.0f, col.g / 300.0f, col.b / 300.0f };
                if (ImGui::ColorEdit3(label.c_str(), tmp)) {
                    col.r = tmp[0] * 300.0f;
                    col.g = tmp[1] * 300.0f;
                    col.b = tmp[2] * 300.0f;
                }
            }
            ImGui::Separator();  // 每个光源分隔一条线
        }
        ImGui::Spacing();
    }

    // —— 3) Camera Settings 区（折叠条） —— 
    if (ImGui::CollapsingHeader("Camera Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        // 调整摄像机位置
        glm::vec3 camPos = m_Camera->Position;
        if (ImGui::DragFloat3("Camera Position", &camPos.x, 0.1f, -20.0f, 20.0f)) {
            m_Camera->Position = camPos;
        }

        // 如果你想让用户直接控制摄像机的 yaw / pitch，也可以如下：
        
        float yaw  = m_Camera->Yaw;
        float pitch = m_Camera->Pitch;
        if (ImGui::SliderFloat("Yaw", &yaw, -180.0f, 180.0f)) {
            m_Camera->Yaw = yaw;
            m_Camera->updateCameraVectors();
        }
        if (ImGui::SliderFloat("Pitch", &pitch, -89.0f, 89.0f)) {
            m_Camera->Pitch = pitch;
            m_Camera->updateCameraVectors();
        }
        
        ImGui::Spacing();
    }

    ImGui::End();
}


void Application::Update(float deltaTime) {
    // 这里可以更新动画、物理。对于 PBR 示例而言，暂时不需要额外逻辑
}

//void Application::Render() {
//    // 直接调用 PBRRenderer 渲染一帧
//    m_PBRRenderer->RenderPBRScene(*m_Camera);
//}

//void Application::Cleanup() {
//    // PBRRenderer 析构会释放 GL 资源
//    m_PBRRenderer.reset();
//    m_Camera.reset();
//    m_InputManager.reset();
//
//    if (m_Window) {
//        m_Window->Shutdown();
//        m_Window.reset();
//    }
//    glfwTerminate();
//}
