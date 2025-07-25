#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#include "mesh.h"
#include "renderer/shader.h"

using namespace std;

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

class Model
{
public:
    vector<Texture> textures_loaded; // 已加载的纹理
    vector<Mesh> meshes;             // 模型的网格
    string directory;                // 模型文件目录
    bool gammaCorrection;            // 是否启用伽马校正

    Model(string const& path, bool gamma = false);
    void Draw(Shader& shader);

private:
    void loadModel(string const& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
};