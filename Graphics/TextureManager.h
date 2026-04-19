#pragma once
#include <lunasvg.h>
#include "XdgUtility.h"
#include "stb_image.h"
#include <iostream>
#include <mutex>

class TextureManager
{

    std::unordered_map<std::string, GLuint> texture_cache_;
    std::mutex m_cache_mutex;
    TextureManager() { stbi_set_flip_vertically_on_load(1); }

    public:
    static TextureManager &Get()
    {
        static TextureManager instance;
        return instance;
    }

    GLuint GetTextureFromFile(const std::string &path)
    {
        if (path.empty()) return 0;

        const std::string full_path = NormalizeTexturePath(path);

        {
            std::scoped_lock lock(m_cache_mutex);
            if (texture_cache_.contains(full_path))
                return texture_cache_[full_path];
        }

        GLuint tex = 0;

        if (full_path.ends_with(".svg")) 
        {
            int w = 0, h = 0;
            if (const auto pixels = LoadSvgToPixels(full_path, 256, w, h); pixels.has_value()) {
                tex = UploadTexture(pixels.value().data(), w, h, true);
            }
        } 
        else
        {
            int w = 0, h = 0, ch = 0;
            if (unsigned char *data = stbi_load(full_path.c_str(), &w, &h, &ch, 4)) {
                tex = UploadTexture(data, w, h, true);
                stbi_image_free(data);
            } else {
                std::cerr << "[TextureManager] Falha ao carregar imagem: " << full_path << std::endl;
            }
        }

        if (tex != 0) 
        {
            std::scoped_lock lock(m_cache_mutex);
            texture_cache_[full_path] = tex;
        }

        return tex;
    }

    // GLuint GetTextureFromMemory(const std::string& unique_name, const unsigned char* buffer, int length) {
    //     if (buffer == nullptr || length == 0) return 0;
    //
    //     if (texture_cache_.contains(unique_name)) return texture_cache_[unique_name];
    //
    //     int w, h, ch;
    //     unsigned char* data = stbi_load_from_memory(buffer, length, &w, &h, &ch, 4);
    //     if (!data) {
    //         std::cerr << "❌ [TextureManager] Falha ao descomprimir memória: " << unique_name << std::endl;
    //         return 0;
    //     }
    //
    //     GLuint tex = UploadTexture(data, w, h);
    //     stbi_image_free(data);
    //
    //     texture_cache_[unique_name] = tex;
    //     return tex;
    // }

    GLuint GetAppIconTexture(const std::string& app_id) {
        const std::string cache_key = "app-icon:" + app_id;
        if (texture_cache_.contains(cache_key)) return texture_cache_[cache_key];

        GLuint tex = 0;
        if (const auto icon_path = XdgEnvironment::GetBestIconPath(app_id); !icon_path.empty())
        {
            tex = GetTextureFromFile(icon_path);
        }

        if (tex == 0)
        {
            tex = GetTextureFromFile("default.png");
        }

        texture_cache_[cache_key] = tex;
        return tex;
    }

    GLuint UploadTexture(const unsigned char *data, int w, int h, bool generate_mipmaps)
    {
        if (!data) return 0;

        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        if (generate_mipmaps) {
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        return tex;
    }

    std::optional<std::vector<unsigned char>> LoadSvgToPixels(const std::string &filepath, int target_size, int &out_w, int &out_h)
    {
        const auto document = lunasvg::Document::loadFromFile(filepath);
        if (!document) return std::nullopt;

        const auto bitmap = document->renderToBitmap(target_size, target_size);
        if (bitmap.isNull()) return std::nullopt;

        out_w = bitmap.width();
        out_h = bitmap.height();

        std::vector pixels(bitmap.data(), bitmap.data() + (out_w * out_h * 4));

        for (size_t i = 0; i < pixels.size(); i += 4) {
            std::swap(pixels[i], pixels[i+2]);
        }

        return pixels;
    }

    static std::string NormalizeTexturePath(const std::string& path) {
        if (path.empty()) return {};
        if (path.starts_with('/')) return path;
        return "/home/talles/Documentos/code/c++/unity/" + path;
    }
};