#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "Shader.h"
#include "core/Camera.h"   

namespace renderer {

    /// 一个简单的光源结构体，包含位置、颜色、强度
    struct Light {
        glm::vec3 Position;
        glm::vec3 Color;
        float     Intensity;

        Light()
            : Position(glm::vec3(0.0f)),
            Color(glm::vec3(1.0f)),
            Intensity(1.0f)
        {}

        Light(const glm::vec3& pos, const glm::vec3& col, float intensity)
            : Position(pos),
            Color(col),
            Intensity(intensity)
        {}
    };

    /// Renderer 基类：
    ///  - 保存屏幕宽高、光源列表
    ///  - 定义延迟渲染两阶段的纯虚接口：GeometryPass() / LightingPass()
    ///  - 提供 LoadShader() 辅助函数，让派生类方便加载 Shader
    class Renderer {
    public:
        /// 构造函数：传入窗口宽高
        Renderer(int width, int height);

        /// 虚析构，方便派生类正确释放资源
        virtual ~Renderer();

        /// 【纯虚】几何阶段（Geometry Pass）：由派生类实现
        /// 将场景中的各个模型渲染到 G-Buffer（位置、法线、Albedo、PBR 参数等）
        virtual void GeometryPass(const core::Camera& camera) = 0;

        /// 【纯虚】光照阶段（Lighting Pass）：由派生类实现
        /// 从 G-Buffer 读取几何信息，结合 PBR (Cook-Torrance) + IBL 计算最终光照
        virtual void LightingPass(const core::Camera& camera) = 0;

        /// 获取当前场景中所有光源（只读）
        const std::vector<Light>& GetLights() const;

    protected:
        /// 屏幕宽度、高度（用来创建 FBO、纹理时指定尺寸）
        int m_ScreenWidth;
        int m_ScreenHeight;

        /// 场景中存在的光源列表
        std::vector<Light> m_Lights;

        /// 辅助函数：加载并编译一个 Shader，返回 Shader 对象
        /// vertexPath：顶点着色器文件路径
        /// fragmentPath：片段着色器文件路径
        /// geometryPath：可选的几何着色器文件路径（如果没有则传 nullptr）
        Shader LoadShader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
    };

} // namespace renderer
