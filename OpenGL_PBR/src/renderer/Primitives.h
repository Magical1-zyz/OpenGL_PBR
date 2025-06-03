#pragma once

#include <vector>
#include <cmath>

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace renderer {

    /**
     * Primitives
     * ----------
     * 提供三个静态函数，用于渲染最常用的几何体：
     *   - RenderSphere(): 渲染一个单位球体（中心在原点，半径 1）
     *   - RenderCube():   渲染一个单位立方体（中心在原点，边长 2）
     *   - RenderQuad():   渲染一个覆盖 NDC 的全屏四边形（XY 平面）
     *
     * 每个函数在第一次调用时会生成 VAO/VBO/EBO 并上传数据，此后重复调用都会复用同一 VAO。
     */
    class Primitives {
    public:
        /// 渲染一个单位球体（中心在原点，半径 1）
        static void RenderSphere();

        /// 渲染一个单位立方体（中心在原点，边长 2，法线和纹理坐标已绑定）
        static void RenderCube();

        /// 渲染一个全屏四边形（覆盖 NDC 平面，纹理坐标 [0,1]×[0,1]）
        static void RenderQuad();

    private:
        // 球体缓存
        static unsigned int   sphereVAO;
        static unsigned int   sphereVBO;
        static unsigned int   sphereEBO;
        static unsigned int   sphereIndexCount;

        // 立方体缓存
        static unsigned int   cubeVAO;
        static unsigned int   cubeVBO;

        // 四边形缓存
        static unsigned int   quadVAO;
        static unsigned int   quadVBO;

        // 以下初始化函数在首次调用对应 Render* 时执行
        static void initSphere();
        static void initCube();
        static void initQuad();
    };

} // namespace renderer
