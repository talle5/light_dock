#pragma once

#include <GLES3/gl3.h>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include "stb_image.h"

namespace nux {

class TextureManager {
private:

    std::unordered_map<std::string, GLuint> texture_cache_;

    TextureManager()
    {
        stbi_set_flip_vertically_on_load(1);
    }

public:

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    static TextureManager& Get() {
        static TextureManager instance;
        return instance;
    }

    GLuint GetTextureFromFile(const std::string& path) {
        if (path.empty()) return 0;
        const std::string base_path = NormalizeTexturePath(path);
        if (texture_cache_.contains(base_path)) return texture_cache_[base_path];

        int w, h, ch;
        unsigned char* data = stbi_load(base_path.c_str(), &w, &h, &ch, 4);
        if (!data) {
            std::cerr << "❌ [TextureManager] Falha ao carregar ficheiro: " <<base_path << std::endl;
            return 0;
        }

        GLuint tex = UploadToGPU(data, w, h);
        stbi_image_free(data);
        
        texture_cache_[base_path] = tex;
        return tex;
    }

    GLuint GetTextureFromMemory(const std::string& unique_name, const unsigned char* buffer, int length) {
        if (buffer == nullptr || length == 0) return 0;

        if (texture_cache_.contains(unique_name)) return texture_cache_[unique_name];

        int w, h, ch;
        unsigned char* data = stbi_load_from_memory(buffer, length, &w, &h, &ch, 4);
        if (!data) {
            std::cerr << "❌ [TextureManager] Falha ao descomprimir memória: " << unique_name << std::endl;
            return 0;
        }

        GLuint tex = UploadToGPU(data, w, h);
        stbi_image_free(data);
        
        texture_cache_[unique_name] = tex;
        return tex;
    }

    GLuint GetAppIconTexture(const std::string& app_id) {
        const std::string cache_key = "app-icon:" + app_id;
        if (texture_cache_.contains(cache_key)) return texture_cache_[cache_key];

        GLuint tex = 0;
        if (const auto icon_path = ResolveAppIconPath(app_id))
        {
            tex = GetTextureFromFile(icon_path->string());
        }

        if (tex == 0)
        {
            tex = GetTextureFromFile("resources/launcher_bfb.png");
        }

        texture_cache_[cache_key] = tex;
        return tex;
    }

private:
    static std::string NormalizeTexturePath(const std::string& path) {
        if (path.empty()) return {};
        if (path.starts_with('/')) return path;
        return "/home/talles/Documentos/code/c++/unity/" + path;
    }

    static std::string Trim(std::string value)
    {
        const auto is_space = [](unsigned char ch) { return std::isspace(ch) != 0; };
        value.erase(value.begin(), std::find_if_not(value.begin(), value.end(), is_space));
        value.erase(std::find_if_not(value.rbegin(), value.rend(), is_space).base(), value.end());
        return value;
    }

    static std::string ToLower(std::string value)
    {
        std::ranges::transform(value, value.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        return value;
    }

    static std::vector<std::filesystem::path> DesktopSearchRoots()
    {
        std::vector<std::filesystem::path> roots{
            "/usr/share/applications",
            "/usr/local/share/applications"
        };

        if (const char* home = std::getenv("HOME"))
        {
            roots.emplace_back(std::string(home) + "/.local/share/applications");
        }

        return roots;
    }

    static std::vector<std::filesystem::path> IconSearchRoots()
    {
        std::vector<std::filesystem::path> roots{
            "/usr/share/icons/hicolor",
            "/usr/share/pixmaps"
        };

        if (const char* home = std::getenv("HOME"))
        {
            roots.emplace_back(std::string(home) + "/.local/share/icons/hicolor");
        }

        return roots;
    }

    static std::optional<std::filesystem::path> FindDesktopFile(const std::string& app_id)
    {
        if (app_id.empty())
        {
            return std::nullopt;
        }

        const std::vector<std::string> candidates{
            app_id + ".desktop",
            ToLower(app_id) + ".desktop"
        };

        for (const auto& root : DesktopSearchRoots())
        {
            for (const auto& candidate : candidates)
            {
                const auto desktop_file = root / candidate;
                if (std::filesystem::exists(desktop_file))
                {
                    return desktop_file;
                }
            }
        }

        return std::nullopt;
    }

    static std::optional<std::string> ReadDesktopIconName(const std::filesystem::path& desktop_file)
    {
        std::ifstream input(desktop_file);
        if (!input.is_open())
        {
            return std::nullopt;
        }

        bool in_desktop_entry = false;
        std::string line;
        while (std::getline(input, line))
        {
            const std::string trimmed = Trim(line);
            if (trimmed.empty() || trimmed.starts_with('#'))
            {
                continue;
            }

            if (trimmed == "[Desktop Entry]")
            {
                in_desktop_entry = true;
                continue;
            }

            if (in_desktop_entry && trimmed.starts_with('['))
            {
                break;
            }

            if (!in_desktop_entry || !trimmed.starts_with("Icon="))
            {
                continue;
            }

            const std::string icon_name = Trim(trimmed.substr(5));
            if (!icon_name.empty())
            {
                return icon_name;
            }
        }

        return std::nullopt;
    }

    static std::optional<std::filesystem::path> ResolveIconNameToFile(const std::string& icon_name)
    {
        if (icon_name.empty())
        {
            return std::nullopt;
        }

        const std::vector<std::string> extensions{".png", ".jpg", ".jpeg"};
        std::filesystem::path icon_path(icon_name);

        if (icon_path.is_absolute())
        {
            if (std::filesystem::exists(icon_path))
            {
                return icon_path;
            }

            for (const auto& ext : extensions)
            {
                icon_path.replace_extension(ext);
                if (std::filesystem::exists(icon_path))
                {
                    return icon_path;
                }
            }

            return std::nullopt;
        }

        for (const auto& root : IconSearchRoots())
        {
            if (!std::filesystem::exists(root))
            {
                continue;
            }

            if (root.filename() == "pixmaps")
            {
                for (const auto& ext : extensions)
                {
                    const auto candidate = root / (icon_name + ext);
                    if (std::filesystem::exists(candidate))
                    {
                        return candidate;
                    }
                }

                continue;
            }

            for (const auto& size_dir : std::filesystem::directory_iterator(root))
            {
                if (!size_dir.is_directory())
                {
                    continue;
                }

                const auto apps_dir = size_dir.path() / "apps";
                if (!std::filesystem::exists(apps_dir))
                {
                    continue;
                }

                for (const auto& ext : extensions)
                {
                    const auto candidate = apps_dir / (icon_name + ext);
                    if (std::filesystem::exists(candidate))
                    {
                        return candidate;
                    }
                }
            }
        }

        return std::nullopt;
    }

    static std::optional<std::filesystem::path> ResolveAppIconPath(const std::string& app_id)
    {
        if (const auto desktop_file = FindDesktopFile(app_id))
        {
            if (const auto icon_name = ReadDesktopIconName(*desktop_file))
            {
                if (const auto icon_path = ResolveIconNameToFile(*icon_name))
                {
                    return icon_path;
                }
            }
        }

        return ResolveIconNameToFile(app_id);
    }

    // Lógica isolada para enviar para o OpenGL
    static GLuint UploadToGPU(const unsigned char* data, int w, int h) {
        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        // Garante suporte a PNGs com transparência
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        // O SEGREDO: Usar apenas GL_LINEAR impede que o OpenGL exija Mipmaps!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Impede que a textura repita e cause bordas estranhas
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        return tex;
    }
};

} // namespace nux
