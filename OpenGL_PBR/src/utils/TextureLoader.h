#pragma once

#include <string>
#include <iostream>

#include <glad/glad.h>


/**
 * TextureLoader
 * -------------
 * 提供静态方法 Load2D，从给定文件路径加载一张 2D 纹理（PNG/JPG 等），
 * 并返回对应的 OpenGL 纹理 ID。内部使用 stb_image 进行图片数据加载。
 *
 * 用法示例：
 *   unsigned int texID = TextureLoader::Load2D("assets/textures/foo.png", true);
 *   // 第二个参数 gamma=true 时可在加载后设置 sRGB 相关格式（若需要）
 */
namespace utils {

    class TextureLoader {
    public:
        /**
         * 从文件 path 加载一张 2D 纹理，返回 OpenGL 纹理 ID。
         * @param path   纹理文件的相对或绝对路径
         * @param gamma  是否对贴图进行 gamma 校正（如果你的 PBR shader 里需要 sRGB）
         * @return       GLuint 纹理 ID；若加载失败，则返回 0
         */
        static unsigned int Load2D(const std::string& path, bool gamma = false);
    };

} // namespace utils