#pragma once

#include <iostream>
#include <vector>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Primitives.h"
#include "core/Camera.h"   
#include "utils/TextureLoader.h"  
#include "imgui/imgui.h"



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
        // void RenderImGui();

        // materials
        void LoadMaterialsFromDirectory(const std::string& parentDirectory);
        void LoadAllMaterials(const std::vector<std::string>& materialNames,
                          const std::string& baseDir);

        /// 给某个球设置材质（只更新 materials[i]）
        void SetMaterialForSphere(int sphereIndex, const std::string& folderPath);

        /// 获取已经加载的材质文件夹名列表
        const std::vector<std::string>& GetMaterialNames() const { return materialNames; }

        /// 查询第 `sphereIndex` 号球当前使用的材质索引
        int  GetSphereMaterialIndex(int sphereIndex) const;

        /// 为第 `sphereIndex` 号球设置材质（传入材质列表中的索引）
        void SetSphereMaterialIndex(int sphereIndex, int materialIndex);

        /// 更换 HDR 环境图
        // void LoadHDRI(const std::string& hdrPath);

        // ********** 新增：让外部可以访问并修改光源 **********
        std::vector<glm::vec3> lightPositions;
        std::vector<glm::vec3> lightColors;

        // ********** 如果还想让外部调整其它参数，也可以暴露出去 **********
        // 例如曝光、gamma 等……
        float exposure = 1.0f;
        float gamma = 2.2f;

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

        std::vector<MaterialTextures> allMaterials;    // 所有扫描到的材质
        std::vector<std::string>      materialNames;   // 对应的文件夹名
        std::vector<MaterialTextures> materials;       // 每个球当前使用的材质
        std::vector<glm::vec3>        materialPositions;
        std::vector<int>              sphereMaterialIdx;  // 每球所选材质 in allMaterials

        // 当前选择的材质和 HDR index
        int selectedMaterialIndex = 0;
        int selectedHDRIndex = 0;

        std::vector<std::string> materialFolders;
        std::vector<std::string> hdrPaths;

        bool useNormalMap = true;
    };

} // namespace renderer
