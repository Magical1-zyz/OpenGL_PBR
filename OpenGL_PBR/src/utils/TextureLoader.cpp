#include "TextureLoader.h"

//#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"


namespace utils {

    unsigned int TextureLoader::Load2D(const std::string& path, bool gamma) {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        // stb_image: 默认从 top-left 读取，需要翻转为 OpenGL 的 bottom-left
        stbi_set_flip_vertically_on_load(true);

        int width, height, nrComponents;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
        if (!data) {
            std::cout << "[TextureLoader] Failed to load texture at path: " << path << std::endl;
            return 0;
        }

        GLenum internalFormat;
        GLenum dataFormat;
        if (nrComponents == 1) {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrComponents == 3) {
            internalFormat = gamma ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4) {
            internalFormat = gamma ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }
        else {
            // 不常见的通道数，使用 RGB 作为默认
            internalFormat = dataFormat = GL_RGB;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            internalFormat,
            width,
            height,
            0,
            dataFormat,
            GL_UNSIGNED_BYTE,
            data
        );
        glGenerateMipmap(GL_TEXTURE_2D);

        // 设置标准参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        return textureID;
    }

} // namespace utils
