#pragma once

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>

namespace XdgEnvironment
{

struct IconSubdir {
    std::string name;
    int size = 0;
    int min_size = 0, max_size = 0, threshold = 2;
    std::string type = "Fixed";
};

struct IconTheme {
    std::vector<std::string> inherits;
    std::vector<IconSubdir> directories;
    std::vector<std::filesystem::path> base_paths;
};

inline std::unordered_map<std::string, IconTheme> g_theme_cache;

inline std::vector<std::string> Split(const std::string& s, char delim) {
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) if (!item.empty()) tokens.push_back(item);
    return tokens;
}

inline std::vector<std::string> GetIconSearchPaths() {
    std::vector<std::string> dirs;
    if (const char *home = std::getenv("HOME")) dirs.push_back(std::string(home) + "/.icons");

    std::string data_home = std::getenv("XDG_DATA_HOME") ? std::getenv("XDG_DATA_HOME") : (std::string(std::getenv("HOME")) + "/.local/share");
    dirs.push_back(data_home + "/icons");

    std::string data_dirs = std::getenv("XDG_DATA_DIRS") ? std::getenv("XDG_DATA_DIRS") : "/usr/local/share:/usr/share";
    for (const auto& d : Split(data_dirs, ':')) dirs.push_back(d + "icons");

    dirs.push_back("/usr/share/pixmaps");
    return dirs;
}

inline void LoadTheme(const std::string& name, const std::vector<std::string>& search_paths) {
    if (name.empty() || g_theme_cache.contains(name)) return;

    IconTheme theme;
    for (const auto& base : search_paths) {
        auto index_path = std::filesystem::path(base) / name / "index.theme";
        if (!std::filesystem::exists(index_path)) continue;

        theme.base_paths.push_back(std::filesystem::path(base) / name);
        std::ifstream file(index_path);
        std::string line, section;
        while (std::getline(file, line)) {
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            if (line.empty() || line[0] == '#') continue;
            if (line[0] == '[') { section = line.substr(1, line.find(']') - 1); continue; }

            auto pos = line.find('=');
            if (pos == std::string::npos) continue;
            std::string key = line.substr(0, pos), val = line.substr(pos + 1);

            if (section == "Icon Theme") {
                if (key == "Inherits") theme.inherits = Split(val, ',');
                if (key == "Directories" && theme.directories.empty())
                    for (auto& d : Split(val, ',')) theme.directories.push_back({.name = d});
            } else {
                for (auto &[name, size, min_size, max_size, threshold, type] : theme.directories) {
                    if (name == section) {
                        if (key == "Size") size = std::stoi(val);
                        if (key == "Type") type = val;
                        if (key == "MinSize") min_size = std::stoi(val);
                        if (key == "MaxSize") max_size = std::stoi(val);
                        if (key == "Threshold") threshold = std::stoi(val);
                    }
                }
            }
        }
    }
    g_theme_cache[name] = theme;
}

inline std::string LookupIcon(const std::string& theme_name, const std::string& icon, int _size, const std::vector<std::string>& search_paths) {
    LoadTheme(theme_name, search_paths);
    if (!g_theme_cache.contains(theme_name)) return "";

    auto &[inherits, directories, base_paths] = g_theme_cache[theme_name];
    for (const auto &[name, size, min_size, max_size, threshold, type] : directories) {
        const bool match = (type == "Fixed" && size == _size) ||
                     (type == "Scalable" && _size >= min_size && _size <= max_size) ||
                     (type == "Threshold" && _size >= (size - threshold) && _size <= (size + threshold));

        if (match) {
            for (const auto& base : base_paths) {
                for (const char* ext : {".svg", ".png", ".xpm"}) {
                    auto p = base / name / (icon + ext);
                    if (std::filesystem::exists(p)) return p.string();
                }
            }
        }
    }
    for (const auto& parent : inherits) {
        std::string p = LookupIcon(parent, icon, _size, search_paths);
        if (!p.empty()) return p;
    }
    return "";
}

inline std::string GetBestIconPath(const std::string& app_id, int size = 48) {
    static const auto search_paths = GetIconSearchPaths();
    std::string icon_name = app_id;

    for (const auto& dir : search_paths) {
        auto p = std::filesystem::path(dir).parent_path() / "applications" / (app_id + ".desktop");
        if (std::filesystem::exists(p)) {
            std::ifstream f(p); std::string line;
            while (std::getline(f, line))
                if (line.starts_with("Icon=")) { icon_name = line.substr(5); break; }
            break;
        }
    }

    std::string path = LookupIcon("Adwaita", icon_name, size, search_paths);
    if (path.empty()) path = LookupIcon("hicolor", icon_name, size, search_paths);
    if (path.empty()) {
        for (const auto& base : search_paths) {
            for (const char* ext : {".svg", ".png", ".xpm"}) {
                auto p = std::filesystem::path(base) / (icon_name + ext);
                if (std::filesystem::exists(p)) return p.string();
            }
        }
    }

    return path.empty() ? "resources/default_app_icon.png" : path;
}

} // namespace XdgEnvironment
