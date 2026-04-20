#pragma once

#include "../launcher/LauncherIcon.h"
#include "Graphics/Renderer2D.h"
#include "Graphics/WindowTracker.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>
#include <wayland-client.h>

struct DockyLayoutConfig {
    float side_padding           = 12.0f;
    float vertical_padding       = 6.0f;
    float icon_size              = 48.0f;
    float space_between_children = 14.0f;
    float outer_margin           = 16.0f;
};

class Docky
{
    static constexpr float kHoverScale    = 1.6f;
    static constexpr float kShadowPadding = 6.0f;

    struct ComputedLayout {
        float dock_x            = 0.0f;
        float dock_y            = 0.0f;
        float dock_width        = 0.0f;
        float dock_height       = 0.0f;
        float icon_spacing      = 0.0f;
        float icons_total_width = 0.0f;
        float icons_x           = 0.0f;
        float icons_y           = 0.0f;
        std::vector<float> icon_sizes;
    };

    std::vector<LauncherIcon> m_icons;
    DockyLayoutConfig m_layout;
    double m_mouse_x = 0.0;
    double m_mouse_y = 0.0;

    public:
    Rect m_geometry;

    [[nodiscard]] static constexpr int RecommendedSurfaceHeight()
    { return static_cast<int>((48.0f * kHoverScale) + (6.0f * 2.0f) + kShadowPadding + 1.0f); }

    Docky() : m_geometry(0, 0, 1, 74)
    {
        LauncherIcon terminal_icon(m_geometry);
        terminal_icon.SetAppId("foot");
        terminal_icon.SetPinned(true);
        AddIcon(terminal_icon);
    }

    void SetGeometry(const Rect &geometry) { m_geometry = geometry; }

    void AddIcon(const LauncherIcon &icon) { m_icons.push_back(icon); }

    [[nodiscard]] bool HasIconForAppId(const std::string &app_id) const
    {
        return std::ranges::any_of(m_icons,
                                   [&](const auto &icon) { return icon.GetAppId() == app_id; });
    }

    void AddTestIconForApp(const std::string &app_id)
    {
        if (app_id.empty() || HasIconForAppId(app_id)) {
            return;
        }

        LauncherIcon icon(m_geometry);
        icon.SetAppId(app_id);
        icon.SetRunning(true);
        AddIcon(icon);
    }

    void UpdateOpenApps(const std::vector<WindowTracker::OpenApp> &open_apps)
    {
        std::erase_if(m_icons, [&](const auto &icon) {
            return !icon.IsPinned() && std::ranges::none_of(open_apps, [&](const auto &app) {
                return app.app_id == icon.GetAppId();
            });
        });

        for (auto &icon : m_icons) {
            icon.SetRunning(false);
        }

        for (const auto &app : open_apps) {
            AddTestIconForApp(app.app_id);

            for (auto &icon : m_icons) {
                if (icon.GetAppId() == app.app_id) {
                    icon.SetRunning(true);
                    break;
                }
            }
        }
    }

    void HandlePointerMotion(double x, double y)
    {
        m_mouse_x = x;
        m_mouse_y = y;

        for (auto &icon : m_icons) {
            icon.SetHovered(icon.geo.IsPointInside(static_cast<int>(x), static_cast<int>(y)));
        }
    }

    void HandlePointerLeave()
    {
        m_mouse_x = 0.0;
        m_mouse_y = 0.0;

        for (auto &icon : m_icons) {
            icon.SetHovered(false);
        }
    }

    void HandlePointerClick(double x, double y, int button, WindowTracker &tracker, wl_seat *seat)
    {
        for (auto &icon : m_icons) {
            if (!icon.geo.IsPointInside(static_cast<int>(x), static_cast<int>(y))) {
                continue;
            }

            if (button == 1) // Botão esquerdo: Abrir/Alternar aplicativo
            {
                icon.OnClick(); // Chama o evento virtual do ícone
                if (!tracker.ToggleMinimizeOrActivateApp(icon.GetAppId(), seat)) {
                    LaunchApp(icon.GetAppId());
                }
            } else if (button == 3) // Botão direito: Abrir menu de contexto
            {
                icon.ToggleMenu();
            }
            break;
        }
    }

    void ComputeLayoutAndDraw(Renderer2D &renderer)
    {
        if (m_geometry.w <= 0 || m_geometry.h <= 0 || m_icons.empty()) {
            return;
        }

        const ComputedLayout layout = CalculateLayout();
        renderer.DrawSquircle(layout.dock_x, layout.dock_y, layout.dock_width, layout.dock_height,
                              0.08f, 0.08f, 0.10f, 0.86f);

        float current_x = layout.icons_x;
        for (size_t i = 0; i < m_icons.size(); ++i) {
            auto &icon            = m_icons[i];
            const float icon_size = layout.icon_sizes[i];
            const float current_y
                = layout.icons_y
                  + (layout.dock_height - (m_layout.vertical_padding * 2.0f) - icon_size) * 0.5f;

            icon.SetPosition(current_x, current_y);
            icon.SetSize(icon_size);
            icon.Draw(renderer);
            current_x += icon_size + layout.icon_spacing;
        }
    }

    private:
    [[nodiscard]] ComputedLayout CalculateLayout() const
    {
        ComputedLayout layout{};
        const size_t count = m_icons.size();
        if (count == 0)
            return layout;

        layout.icon_sizes.resize(count);

        const float base_size = m_layout.icon_size;
        const float max_size  = base_size * kHoverScale;

        const float spacing = (count > 1) ? m_layout.space_between_children : 0.0f;

        const float surface_center_x
            = static_cast<float>(m_geometry.x) + (static_cast<float>(m_geometry.w) * 0.5f);

        // --- 1. posição base dos ícones (sem escala) ---
        const float base_total_width = (count * base_size) + ((count - 1.0f) * spacing);

        float start_x = surface_center_x - (base_total_width * 0.5f);

        // --- 2. calcular centros reais em pixels ---
        std::vector<float> centers(count);

        float cursor = start_x;
        for (size_t i = 0; i < count; ++i) {
            centers[i]  = cursor + base_size * 0.5f;
            cursor     += base_size + spacing;
        }

        // --- 3. calcular escala com distância REAL ---
        float total_width      = 0.0f;
        float max_current_size = base_size;

        for (size_t i = 0; i < count; ++i) {
            constexpr float sigma = 80.0f;
            const float dist      = std::abs(m_mouse_x - centers[i]);

            // gaussiano
            const float influence = std::exp(-(dist * dist) / (2.0f * sigma * sigma));

            const float size = base_size + (max_size - base_size) * influence;

            layout.icon_sizes[i]  = size;
            total_width          += size;

            if (size > max_current_size)
                max_current_size = size;
        }

        total_width += (count - 1.0f) * spacing;

        // --- 4. layout final centralizado ---
        layout.icons_total_width = total_width;
        layout.dock_width        = total_width + (m_layout.side_padding * 2.0f);
        layout.dock_height       = max_current_size + (m_layout.vertical_padding * 2.0f);

        layout.icons_x = surface_center_x - (total_width * 0.5f);
        layout.dock_x  = surface_center_x - (layout.dock_width * 0.5f);
        layout.dock_y  = static_cast<float>(m_geometry.y + m_geometry.h) - layout.dock_height
                        - kShadowPadding;

        layout.icons_y = layout.dock_y + m_layout.vertical_padding;

        layout.icon_spacing = spacing;

        return layout;
    }

    static bool IsSafeCommandToken(const std::string_view app_id)
    {
        return !app_id.empty() && std::ranges::all_of(app_id, [](const unsigned char ch) {
            return std::isalnum(ch) || ch == '-' || ch == '_' || ch == '.';
        });
    }

    static void LaunchApp(const std::string_view app_id)
    {
        if (!IsSafeCommandToken(app_id)) {
            return;
        }

        const std::string command = std::string(app_id) + " >/dev/null 2>&1 &";
        std::system(command.c_str());
    }
};
