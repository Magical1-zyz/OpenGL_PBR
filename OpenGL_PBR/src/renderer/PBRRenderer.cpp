#include "PBRRenderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"


namespace renderer
{
    namespace fs = std::filesystem;

    PBRRenderer::PBRRenderer(unsigned int width, unsigned int height)
        : SCR_WIDTH(width), SCR_HEIGHT(height),
          pbrShader("assets/shaders/pbrShader/pbr.vert", "assets/shaders/pbrShader/pbr.frag"),
          equirectangularToCubemapShader(
              "assets/shaders/equirectangularToCubemapShader/equirectangularToCubemap.vert",
              "assets/shaders/equirectangularToCubemapShader/equirectangularToCubemap.frag"
          ),
          irradianceShader("assets/shaders/irradianceShader/irradiance.vert",
                           "assets/shaders/irradianceShader/irradiance.frag"),
          prefilterShader("assets/shaders/prefilterShader/prefilter.vert",
                          "assets/shaders/prefilterShader/prefilter.frag"),
          brdfShader("assets/shaders/brdfShader/brdf.vert", "assets/shaders/brdfShader/brdf.frag"),
          backgroundShader("assets/shaders/backgroundShader/background.vert",
                           "assets/shaders/backgroundShader/background.frag"),
          hdrTexture(0),
          envCubemap(0),
          irradianceMap(0),
          prefilterMap(0),
          brdfLUTTexture(0),
          captureFBO(0),
          captureRBO(0)
    {
        // 在构造里只做简单的成员初始化，不开显存
    }

    PBRRenderer::~PBRRenderer()
    {
        // 释放所有 OpenGL 资源
        glDeleteFramebuffers(1, &captureFBO);
        glDeleteRenderbuffers(1, &captureRBO);
        glDeleteTextures(1, &hdrTexture);
        glDeleteTextures(1, &envCubemap);
        glDeleteTextures(1, &irradianceMap);
        glDeleteTextures(1, &prefilterMap);
        glDeleteTextures(1, &brdfLUTTexture);
    }

    /// 在 Application 初始化时调用，完成一次性预计算
    void PBRRenderer::InitPBR(const std::string& hdrPath)
    {
        // ------------------------------------------------------------------------
        //  1. 创建 captureFBO、captureRBO，用于后续各次 render 到立方体贴图
        // ------------------------------------------------------------------------
        glGenFramebuffers(1, &captureFBO);
        glGenRenderbuffers(1, &captureRBO);

        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

        // ------------------------------------------------------------------------
        //  2. 加载 HDR 环境图到 2D 纹理
        // ------------------------------------------------------------------------
        stbi_set_flip_vertically_on_load(true);
        int width, height, nrComponents;
        float* data = stbi_loadf(hdrPath.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            glGenTextures(1, &hdrTexture);
            glBindTexture(GL_TEXTURE_2D, hdrTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "[PBRRenderer] Failed to load HDR image: " << hdrPath << std::endl;
        }

        // ------------------------------------------------------------------------
        //  3. 创建一个空的立方体贴图 envCubemap，用于存放从 HDRMap 转换来的 cubemap
        // ------------------------------------------------------------------------
        glGenTextures(1, &envCubemap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
        for (unsigned int i = 0; i < 6; ++i)
        {
            // 512×512 16F 每个面
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB16F, 512, 512, 0,
                GL_RGB, GL_FLOAT, nullptr
            );
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // ------------------------------------------------------------------------
        //  4. 用 equirectangularToCubemapShader 把 HDR 2D 图渲染到 6 个面上
        // ------------------------------------------------------------------------
        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[] = {
            glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
        };

        equirectangularToCubemapShader.use();
        equirectangularToCubemapShader.setInt("equirectangularMap", 0);
        equirectangularToCubemapShader.setMat4("projection", captureProjection);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);

        glViewport(0, 0, 512, 512);
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        for (unsigned int i = 0; i < 6; ++i)
        {
            equirectangularToCubemapShader.setMat4("view", captureViews[i]);
            glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                envCubemap,
                0
            );
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            Primitives::RenderCube();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 生成 mipmap
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        // ------------------------------------------------------------------------
        //  5. 生成 32×32 的 irradianceMap
        // ------------------------------------------------------------------------
        glGenTextures(1, &irradianceMap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB16F, 32, 32, 0,
                GL_RGB, GL_FLOAT, nullptr
            );
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

        irradianceShader.use();
        irradianceShader.setInt("environmentMap", 0);
        irradianceShader.setMat4("projection", captureProjection);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

        glViewport(0, 0, 32, 32);
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        for (unsigned int i = 0; i < 6; ++i)
        {
            irradianceShader.setMat4("view", captureViews[i]);
            glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                irradianceMap,
                0
            );
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            Primitives::RenderCube();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ------------------------------------------------------------------------
        //  6. 生成 128×128~ 的 prefilterMap，并逐层渲染
        // ------------------------------------------------------------------------
        glGenTextures(1, &prefilterMap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB16F, 128, 128, 0,
                GL_RGB, GL_FLOAT, nullptr
            );
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        prefilterShader.use();
        prefilterShader.setInt("environmentMap", 0);
        prefilterShader.setMat4("projection", captureProjection);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        unsigned int maxMipLevels = 5;
        for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
        {
            unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
            unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
            glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
            glViewport(0, 0, mipWidth, mipHeight);

            float roughness = (float)mip / (float)(maxMipLevels - 1);
            prefilterShader.setFloat("roughness", roughness);
            for (unsigned int i = 0; i < 6; ++i)
            {
                prefilterShader.setMat4("view", captureViews[i]);
                glFramebufferTexture2D(
                    GL_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0,
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    prefilterMap,
                    mip
                );
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                Primitives::RenderCube();
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ------------------------------------------------------------------------
        //  7. 生成 512×512 BRDF LUT
        // ------------------------------------------------------------------------
        glGenTextures(1, &brdfLUTTexture);
        glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RG16F,
            512, 512,
            0, GL_RG, GL_FLOAT, nullptr
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D,
            brdfLUTTexture,
            0
        );
        glViewport(0, 0, 512, 512);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        brdfShader.use();
        Primitives::RenderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ------------------------------------------------------------------------
        //  8. 配置 pbrShader 和 backgroundShader 中的常量
        // ------------------------------------------------------------------------
        pbrShader.use();
        pbrShader.setInt("irradianceMap", 0);
        pbrShader.setInt("prefilterMap", 1);
        pbrShader.setInt("brdfLUT", 2);
        pbrShader.setInt("albedoMap", 3);
        pbrShader.setInt("normalMap", 4);
        pbrShader.setInt("metallicMap", 5);
        pbrShader.setInt("roughnessMap", 6);
        pbrShader.setInt("aoMap", 7);

        backgroundShader.use();
        backgroundShader.setInt("environmentMap", 0);

        // ------------------------------------------------------------------------
        //  9. 初始化几种材料贴图和光源
        // ------------------------------------------------------------------------
        // 假设你有几种 PBR 材质放在 assets/textures/pbr/<name>/ 里
        // 先预载它们：
        std::string baseDir = "assets/textures/pbr/";
        std::vector<std::string> names = {"rusted_iron", "gold", "grass", "plastic", "wall"};
        for (const auto& n : names)
        {
            MaterialTextures mat;
            mat.albedo = utils::TextureLoader::Load2D((baseDir + n + "/albedo.png").c_str());
            mat.normal = utils::TextureLoader::Load2D((baseDir + n + "/normal.png").c_str());
            mat.metallic = utils::TextureLoader::Load2D((baseDir + n + "/metallic.png").c_str());
            mat.roughness = utils::TextureLoader::Load2D((baseDir + n + "/roughness.png").c_str());
            mat.ao = utils::TextureLoader::Load2D((baseDir + n + "/ao.png").c_str());
            materials.push_back(mat);
        }
        // 对应的球体位置
        materialPositions = {
            {-5.0f, 0.0f, 2.0f},
            {-3.0f, 0.0f, 2.0f},
            {-1.0f, 0.0f, 2.0f},
            {1.0f, 0.0f, 2.0f},
            {3.0f, 0.0f, 2.0f}
        };

        // 光源
        lightPositions = {
            {-10.0f, 10.0f, 10.0f},
            {10.0f, 10.0f, 10.0f},
            {-10.0f, -10.0f, 10.0f},
            {10.0f, -10.0f, 10.0f}
        };
        lightColors = {
            {300.0f, 300.0f, 300.0f},
            {300.0f, 300.0f, 300.0f},
            {300.0f, 300.0f, 300.0f},
            {300.0f, 300.0f, 300.0f}
        };

        // 进入最后阶段之前，切换视口回原始尺寸
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        int scrW, scrH;
        glfwGetFramebufferSize(glfwGetCurrentContext(), &scrW, &scrH);
        glViewport(0, 0, scrW, scrH);
    }

    /// 每帧调用此函数，使用当前相机渲染一次完整的 PBR 场景
    void PBRRenderer::RenderPBRScene(const core::Camera& camera)
    {
        // 1. 先清屏
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 2. 计算 view、projection 矩阵
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(
            glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f, 100.0f
        );

        // 3. 绘制 PBR 球体（及光源小球），保持默认深度设置
        //    深度测试已在 Window 初始化时 glEnable(GL_DEPTH_TEST) 并设为 GL_LEQUAL/GL_LESS
        pbrShader.use();
        pbrShader.setMat4("view", view);
        pbrShader.setMat4("projection", projection);
        pbrShader.setVec3("camPos", camera.Position);

        // 绑定预计算的 IBL 数据
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

        // 依次绘制每个 PBR 球体，绑定它对应材质贴图
        for (size_t i = 0; i < materials.size(); ++i)
        {
            auto &mat = materials[i];
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, mat.albedo);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, mat.normal);
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, mat.metallic);
            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_2D, mat.roughness);
            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_2D, mat.ao);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, materialPositions[i]);
            pbrShader.setMat4("model", model);
            pbrShader.setMat3(
                "normalMatrix",
                glm::transpose(glm::inverse(glm::mat3(model)))
            );
            Primitives::RenderSphere();
        }

        // 4. 渲染“光源”小球
        for (size_t i = 0; i < lightPositions.size(); ++i)
        {
            glm::vec3 newPos = lightPositions[i];
            pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
            pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, newPos);
            model = glm::scale(model, glm::vec3(0.5f));
            pbrShader.setMat4("model", model);
            pbrShader.setMat3(
                "normalMatrix",
                glm::transpose(glm::inverse(glm::mat3(model)))
            );
            Primitives::RenderSphere();
        }

        // 5. 渲染天空盒（背景立方体贴图）
        //    a) 关闭深度写入，让天空盒永远绘制在最远处；
        //    b) 使用去掉平移分量的 view 矩阵。

        // 5.1 关闭深度写入
        glDepthMask(GL_FALSE);
        // 可选：确保深度函数为 “小于或等于”。
        // 如果之前设置的是 GL_LESS，也可以在此改为 GL_LEQUAL：
        glDepthFunc(GL_LEQUAL);

        backgroundShader.use();
        // 去掉 view 中的平移成分：只保留旋转部分
        glm::mat4 viewNoTranslate = glm::mat4(glm::mat3(view));
        backgroundShader.setMat4("view", viewNoTranslate);
        backgroundShader.setMat4("projection", projection);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
        Primitives::RenderCube();

        // 5.2 恢复深度写入和深度函数
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }

    /// 处理窗口大小变化
    void PBRRenderer::Resize(unsigned int width, unsigned int height)
    {
        SCR_WIDTH = width;
        SCR_HEIGHT = height;
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    }


    void PBRRenderer::LoadMaterialsFromDirectory(const std::string& parentDirectory)
    {
        materials.clear();
        materialPositions.clear();

        // 1) 遍历 parentDirectory 下的每个子目录
        if (!fs::exists(parentDirectory) || !fs::is_directory(parentDirectory))
        {
            std::cerr << "[PBRRenderer] Invalid materials directory: " << parentDirectory << std::endl;
            return;
        }

        std::vector<std::string> subdirs;
        for (auto& entry : fs::directory_iterator(parentDirectory))
        {
            if (entry.is_directory())
            {
                subdirs.push_back(entry.path().string());
            }
        }
        if (subdirs.empty())
        {
            std::cerr << "[PBRRenderer] No material subfolders found in " << parentDirectory << std::endl;
            return;
        }

        // 2) 对每个子目录依次加载贴图，存入 materials
        for (auto& folderPath : subdirs)
        {
            MaterialTextures mat;
            std::string base = folderPath + "/";

            mat.albedo = utils::TextureLoader::Load2D((base + "albedo.png").c_str());
            mat.normal = utils::TextureLoader::Load2D((base + "normal.png").c_str());
            mat.metallic = utils::TextureLoader::Load2D((base + "metallic.png").c_str());
            mat.roughness = utils::TextureLoader::Load2D((base + "roughness.png").c_str());
            mat.ao = utils::TextureLoader::Load2D((base + "ao.png").c_str());

            materials.push_back(mat);
        }

        // 3) 自动生成 materialPositions，使球体水平排列
        size_t N = materials.size();
        float spacing = 2.5f;
        // 从 -(N-1)/2 到 +(N-1)/2
        for (size_t i = 0; i < N; ++i)
        {
            float x = (float(i) - float(N - 1) * 0.5f) * spacing;
            materialPositions.push_back(glm::vec3(x, 0.0f, 2.0f));
        }

        std::cout << "[PBRRenderer] Loaded " << N << " materials from " << parentDirectory << std::endl;
    }

    // 初始化时调用一次：把所有子文件夹的贴图 Load 进 allMaterials
    void PBRRenderer::LoadAllMaterials(
        const std::vector<std::string>& names,
        const std::string& baseDir
    )
    {
        materialNames = names;
        allMaterials.clear();

        for (auto& n : names)
        {
            MaterialTextures mat;
            std::string folder = baseDir + "/" + n + "/";
            mat.albedo = utils::TextureLoader::Load2D((folder + "albedo.png").c_str());
            mat.normal = utils::TextureLoader::Load2D((folder + "normal.png").c_str());
            mat.metallic = utils::TextureLoader::Load2D((folder + "metallic.png").c_str());
            mat.roughness = utils::TextureLoader::Load2D((folder + "roughness.png").c_str());
            mat.ao = utils::TextureLoader::Load2D((folder + "ao.png").c_str());
            allMaterials.push_back(mat);
        }

        // 为五个球准备位置（水平排列）和初始材质索引 0
        int N = 5;
        float spacing = 2.5f;
        materials.resize(N);
        materialPositions.resize(N);
        sphereMaterialIdx.assign(N, 0);
        for (int i = 0; i < N; ++i)
        {
            materialPositions[i] = glm::vec3((i - (N - 1) / 2.0f) * spacing, 0.0f, 2.0f);
            materials[i] = allMaterials[0]; // 默认第一个材质
        }
    }


    // 单独设置某个球的材质
    void PBRRenderer::SetMaterialForSphere(int sphereIndex, const std::string& folderPath)
    {
        // 找 folderPath 在 materialNames 中的索引
        auto it = std::find(materialNames.begin(), materialNames.end(), folderPath);
        if (it == materialNames.end()) return;
        int idx = int(std::distance(materialNames.begin(), it));
        if (sphereIndex < 0 || sphereIndex >= (int)materials.size()) return;
        sphereMaterialIdx[sphereIndex] = idx;
        materials[sphereIndex] = allMaterials[idx];
    }

    int PBRRenderer::GetSphereMaterialIndex(int sphereIndex) const {
        if (sphereIndex < 0 || sphereIndex >= (int)sphereMaterialIdx.size()) {
            std::cerr << "[PBRRenderer] GetSphereMaterialIndex: invalid sphereIndex=" 
                      << sphereIndex << std::endl;
            return 0;
        }
        return sphereMaterialIdx[sphereIndex];
    }

    void PBRRenderer::SetSphereMaterialIndex(int sphereIndex, int materialIndex) {
        if (sphereIndex < 0 || sphereIndex >= (int)sphereMaterialIdx.size()) {
            std::cerr << "[PBRRenderer] SetSphereMaterialIndex: invalid sphereIndex=" 
                      << sphereIndex << std::endl;
            return;
        }
        if (materialIndex < 0 || materialIndex >= (int)allMaterials.size()) {
            std::cerr << "[PBRRenderer] SetSphereMaterialIndex: invalid materialIndex=" 
                      << materialIndex << std::endl;
            return;
        }
        sphereMaterialIdx[sphereIndex] = materialIndex;
        materials[sphereIndex]        = allMaterials[materialIndex];
    }
}// namespace renderer
