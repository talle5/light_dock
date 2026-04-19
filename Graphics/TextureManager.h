#pragma once
#include "../nanosvg/nanosvg.h"
#include "../nanosvg/nanosvgrast.h"
#include "XdgUtility.h"
#include "stb_image.h"
#include <iostream>
#include <mutex>

class TextureManager
{

    std::unordered_map<std::string, GLuint> texture_cache_;
    std::mutex m_cache_mutex; // O Cadeado para permitir o uso em Worker Threads!
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

        const std::string full_path = NormalizeTexturePath(path); // Assumindo que isto vem do seu XdgUtility

        {
            std::scoped_lock lock(m_cache_mutex);
            if (texture_cache_.contains(full_path))
                return texture_cache_[full_path];
        }

        GLuint tex = 0;

        if (full_path.ends_with(".svg")) 
        {
            int w = 0, h = 0;
            // Pedimos ao SVG para ser rasterizado em 256x256 (tamanho premium para docas)
            auto pixels = LoadSvgToPixels(full_path, 256, w, h);
            if (pixels.has_value()) {
                tex = UploadTexture(pixels.value().data(), w, h, true);
            }
        } 
        else
        {
            int w = 0, h = 0, ch = 0;
            unsigned char *data = stbi_load(full_path.c_str(), &w, &h, &ch, 4);
            if (data) {
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
        NSVGimage *image = nsvgParseFromFile(filepath.c_str(), "px", 96.0f);
        if (!image) return std::nullopt;

        NSVGrasterizer *rast = nsvgCreateRasterizer();
        if (!rast) {
            nsvgDelete(image); // Evita leak aqui também!
            return std::nullopt;
        }

        float scale = static_cast<float>(target_size) / std::max(image->width, image->height);
        out_w = static_cast<int>(image->width * scale);
        out_h = static_cast<int>(image->height * scale);

        std::vector<unsigned char> pixels(out_w * out_h * 4);
        
        // Preenche o Vector com a imagem rasterizada
        nsvgRasterize(rast, image, 0, 0, scale, pixels.data(), out_w, out_h, out_w * 4);

        // A CORREÇÃO DE OURO: Limpar a memória do NanoSVG!
        nsvgDeleteRasterizer(rast);
        nsvgDelete(image);

        return pixels; // O std::optional usa move semantics por padrão, zero custo de cópia!
    }

    static std::string NormalizeTexturePath(const std::string& path) {
        if (path.empty()) return {};
        if (path.starts_with('/')) return path;
        return "/home/talles/Documentos/code/c++/unity/" + path;
    }
};