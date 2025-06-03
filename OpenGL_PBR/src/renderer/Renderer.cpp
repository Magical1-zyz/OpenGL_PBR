#include "Renderer.h"

namespace renderer {

    Renderer::Renderer(int width, int height)
        : m_ScreenWidth(width),
        m_ScreenHeight(height)
    {
        // 基类构造：仅保存窗口尺寸
        // 派生类可在自己的构造中向 m_Lights 中添加初始光源
    }

    Renderer::~Renderer() {
        // 基类析构：如果有公用资源（如公用纹理或 FBO）需要释放，可以放在这里
        // 本示例中并无需要基类统一释放的资源，派生类自行管理自己的 OpenGL 资源
    }

    const std::vector<Light>& Renderer::GetLights() const {
        return m_Lights;
    }

    Shader Renderer::LoadShader(const char* vertexPath, const char* fragmentPath, const char* geometryPath) {
        // 直接调用你提供的 Shader 构造函数
        return Shader(vertexPath, fragmentPath, geometryPath);
    }

} // namespace renderer
