#pragma once

#include <memory>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "core/Window.h"
#include "core/InputManager.h"
#include "core/Camera.h"
#include "renderer/PBRRenderer.h"

class Application {
public:
    Application(int width, int height, const std::string& title);
    ~Application();

    void Run();

private:
    void InitWindow();
    void InitSystems();
    void Update(float deltaTime);
    void Render();
    void Cleanup();

private:
    int m_ScreenWidth, m_ScreenHeight;
    std::string m_WindowTitle;

    std::unique_ptr<core::Window>       m_Window;
    std::unique_ptr<core::InputManager> m_InputManager;
    std::unique_ptr<core::Camera>       m_Camera;
    std::unique_ptr<renderer::PBRRenderer> m_PBRRenderer;

    float m_LastFrameTime = 0.0f;
};
