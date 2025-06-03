#pragma once

#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Primitives.h"
#include "core/Camera.h"   
#include "utils/TextureLoader.h"  



namespace renderer {


    /// PBRRenderer: 专门负责整条 PBR + IBL 管线
    /// 包括：
    ///   1) 从 HDR 图生成环境立方体贴图、辐照图、预滤波立方体贴图、BRDF LUT
    ///   2) 渲染 PBR 球体（不同材质）
    ///   3) 渲染光源立方体、背景贴图（天空盒）
    ///   4) 后期如果需要可以扩展 SSAO、Bloom 等
    class PBRRenderer {
    public:
        PBRRenderer(unsigned int width, unsigned int height);
        ~PBRRenderer();

        /// 在 Application 初始化时调用，一次性完成各个 framebuffer / 纹理的创建与预计算
        void InitPBR(const std::string& hdrPath);

        /// 每帧调用，给定当前摄像机，执行一次 PBR 渲染（填充屏幕）
        void RenderPBRScene(const core::Camera& camera);

        /// 处理窗口大小变化
        void Resize(unsigned int width, unsigned int height);

        /// ImGui 面板：调用此函数将渲染 GUI 控件
        void RenderImGui();

        /// 更换材质贴图目录
        void LoadMaterialFromFolder(const std::string& folderPath);

        /// 更换 HDR 环境图
        void LoadHDRI(const std::string& hdrPath);

    private:
        unsigned int SCR_WIDTH, SCR_HEIGHT;

        // ------------------------------------------------------------
        // 1. Shader 对象
        Shader pbrShader;
        Shader equirectangularToCubemapShader;
        Shader irradianceShader;
        Shader prefilterShader;
        Shader brdfShader;
        Shader backgroundShader;

        // ------------------------------------------------------------
        // 2. PBR 所需帧缓冲和贴图
        unsigned int captureFBO, captureRBO;
        unsigned int hdrTexture;      // HDR 环境图
        unsigned int envCubemap;      // Equirectangular 转 cubemap 的结果
        unsigned int irradianceMap;   // 32×32 辐照图
        unsigned int prefilterMap;    // 128×128~的预滤波立方体贴图
        unsigned int brdfLUTTexture;  // 512×512 BRDF LUT

        // ------------------------------------------------------------
        // 3. PBR 材质贴图（Albedo、Normal、Metallic、Roughness、AO）
        struct MaterialTextures {
            unsigned int albedo;
            unsigned int normal;
            unsigned int metallic;
            unsigned int roughness;
            unsigned int ao;
        };
        std::vector<MaterialTextures> materials;
        std::vector<glm::vec3> materialPositions;

        // 当前选择的材质和 HDR index
        int selectedMaterialIndex = 0;
        int selectedHDRIndex = 0;

        std::vector<std::string> materialFolders;
        std::vector<std::string> hdrPaths;

        // ------------------------------------------------------------
        // 4. 光源位置与颜色
        std::vector<glm::vec3> lightPositions;
        std::vector<glm::vec3> lightColors;

        // ------------------------------------------------------------
        // 5. ImGui 可调参数
        float exposure = 1.0f;
        float gamma = 2.2f;
        bool useNormalMap = true;
    };

} // namespace renderer
