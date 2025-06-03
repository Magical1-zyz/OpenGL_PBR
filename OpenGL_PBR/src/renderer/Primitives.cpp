#include "Primitives.h"


namespace renderer {

    // 静态成员初始化
    unsigned int Primitives::sphereVAO = 0;
    unsigned int Primitives::sphereVBO = 0;
    unsigned int Primitives::sphereEBO = 0;
    unsigned int Primitives::sphereIndexCount = 0;

    unsigned int Primitives::cubeVAO = 0;
    unsigned int Primitives::cubeVBO = 0;

    unsigned int Primitives::quadVAO = 0;
    unsigned int Primitives::quadVBO = 0;

#pragma region Sphere

    void Primitives::RenderSphere() {
        if (sphereVAO == 0) {
            initSphere();
        }
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(sphereIndexCount), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void Primitives::initSphere() {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
            if (!oddRow) {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else {
                for (int x = X_SEGMENTS; x >= 0; --x) {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        sphereIndexCount = static_cast<unsigned int>(indices.size());

        std::vector<float> data;
        data.reserve(positions.size() * 8);
        for (size_t i = 0; i < positions.size(); ++i) {
            // 位置
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            // 法线
            data.push_back(normals[i].x);
            data.push_back(normals[i].y);
            data.push_back(normals[i].z);
            // 纹理坐标
            data.push_back(uv[i].x);
            data.push_back(uv[i].y);
        }

        glGenVertexArrays(1, &sphereVAO);
        glGenBuffers(1, &sphereVBO);
        glGenBuffers(1, &sphereEBO);

        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
        glBufferData(
            GL_ARRAY_BUFFER,
            data.size() * sizeof(float),
            data.data(),
            GL_STATIC_DRAW
        );
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            indices.size() * sizeof(unsigned int),
            indices.data(),
            GL_STATIC_DRAW
        );

        // stride = 3（位置）+3（法线）+2（UV） = 8 floats
        GLsizei stride = 8 * sizeof(float);
        // aPos (location = 0)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        // aNormal (location = 1)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        // aTexCoords (location = 2)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));

        glBindVertexArray(0);
    }

#pragma endregion Sphere

#pragma region Cube

    void Primitives::RenderCube() {
        if (cubeVAO == 0) {
            initCube();
        }
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }

    void Primitives::initCube() {
        // 36 个顶点，每个顶点：位置(3)、法线(3)、UV(2) 共 8 floats
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
             1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
             1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
             1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
            -1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,
            // front face
            -1.0f, -1.0f,  1.0f,   0.0f,  0.0f, 1.0f,    0.0f, 0.0f,
             1.0f, -1.0f,  1.0f,   0.0f,  0.0f, 1.0f,    1.0f, 0.0f,
             1.0f,  1.0f,  1.0f,   0.0f,  0.0f, 1.0f,    1.0f, 1.0f,
             1.0f,  1.0f,  1.0f,   0.0f,  0.0f, 1.0f,    1.0f, 1.0f,
            -1.0f,  1.0f,  1.0f,   0.0f,  0.0f, 1.0f,    0.0f, 1.0f,
            -1.0f, -1.0f,  1.0f,   0.0f,  0.0f, 1.0f,    0.0f, 0.0f,
            // left face
            -1.0f,  1.0f,  1.0f,  -1.0f,  0.0f, 0.0f,    1.0f, 0.0f,
            -1.0f,  1.0f, -1.0f,  -1.0f,  0.0f, 0.0f,    1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,  -1.0f,  0.0f, 0.0f,    0.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,  -1.0f,  0.0f, 0.0f,    0.0f, 1.0f,
            -1.0f, -1.0f,  1.0f,  -1.0f,  0.0f, 0.0f,    0.0f, 0.0f,
            -1.0f,  1.0f,  1.0f,  -1.0f,  0.0f, 0.0f,    1.0f, 0.0f,
            // right face
             1.0f,  1.0f,  1.0f,   1.0f,  0.0f, 0.0f,    1.0f, 0.0f,
             1.0f, -1.0f, -1.0f,   1.0f,  0.0f, 0.0f,    0.0f, 1.0f,
             1.0f,  1.0f, -1.0f,   1.0f,  0.0f, 0.0f,    1.0f, 1.0f,
             1.0f, -1.0f, -1.0f,   1.0f,  0.0f, 0.0f,    0.0f, 1.0f,
             1.0f,  1.0f,  1.0f,   1.0f,  0.0f, 0.0f,    1.0f, 0.0f,
             1.0f, -1.0f,  1.0f,   1.0f,  0.0f, 0.0f,    0.0f, 0.0f,
             // bottom face
             -1.0f, -1.0f, -1.0f,   0.0f, -1.0f, 0.0f,    0.0f, 1.0f,
              1.0f, -1.0f, -1.0f,   0.0f, -1.0f, 0.0f,    1.0f, 1.0f,
              1.0f, -1.0f,  1.0f,   0.0f, -1.0f, 0.0f,    1.0f, 0.0f,
              1.0f, -1.0f,  1.0f,   0.0f, -1.0f, 0.0f,    1.0f, 0.0f,
             -1.0f, -1.0f,  1.0f,   0.0f, -1.0f, 0.0f,    0.0f, 0.0f,
             -1.0f, -1.0f, -1.0f,   0.0f, -1.0f, 0.0f,    0.0f, 1.0f,
             // top face
             -1.0f,  1.0f, -1.0f,   0.0f, 1.0f, 0.0f,     0.0f, 1.0f,
              1.0f,  1.0f,  1.0f,   0.0f, 1.0f, 0.0f,     1.0f, 0.0f,
              1.0f,  1.0f, -1.0f,   0.0f, 1.0f, 0.0f,     1.0f, 1.0f,
              1.0f,  1.0f,  1.0f,   0.0f, 1.0f, 0.0f,     1.0f, 0.0f,
             -1.0f,  1.0f, -1.0f,   0.0f, 1.0f, 0.0f,     0.0f, 1.0f,
             -1.0f,  1.0f,  1.0f,   0.0f, 1.0f, 0.0f,     0.0f, 0.0f
        };

        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);

        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindVertexArray(cubeVAO);
        // aPos (location = 0)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE,
            8 * sizeof(float),
            (void*)0
        );
        // aNormal (location = 1)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1, 3, GL_FLOAT, GL_FALSE,
            8 * sizeof(float),
            (void*)(3 * sizeof(float))
        );
        // aTexCoords (location = 2)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(
            2, 2, GL_FLOAT, GL_FALSE,
            8 * sizeof(float),
            (void*)(6 * sizeof(float))
        );
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

#pragma endregion Cube

#pragma region Quad

    void Primitives::RenderQuad() {
        if (quadVAO == 0) {
            initQuad();
        }
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

    void Primitives::initQuad() {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
             1.0f,  1.0f, 0.0f,   1.0f, 1.0f,
             1.0f, -1.0f, 0.0f,   1.0f, 0.0f
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(quadVertices),
            quadVertices,
            GL_STATIC_DRAW
        );

        // aPos (location = 0)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE,
            5 * sizeof(float),
            (void*)0
        );
        // aTexCoords (location = 1)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1, 2, GL_FLOAT, GL_FALSE,
            5 * sizeof(float),
            (void*)(3 * sizeof(float))
        );
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

#pragma endregion Quad

} // namespace renderer
