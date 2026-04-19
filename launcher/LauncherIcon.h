#ifndef SIMPLELAUNCHERICON_H
#define SIMPLELAUNCHERICON_H

#include "../Graphics/Renderer2D.h"
#include "../Graphics/TextureManager.h"
#include "Icon.h"
#include <GLES3/gl3.h>

namespace unity::shell
{
class LauncherIcon : public Icon
{
    protected:
    bool m_menu_open = false;
    bool m_is_pinned = false;

    public:
    bool m_textures_loaded = false;
    GLuint tex_logo{0};

    explicit LauncherIcon(const nux::Rect &g) : Icon(g) {}

    void SetPinned(bool pinned) { m_is_pinned = pinned; }
    [[nodiscard]] bool IsPinned() const { return m_is_pinned; }

    void LoadAssets()
    {
        auto &tm = nux::TextureManager::Get();
        tex_logo = tm.GetAppIconTexture(m_appId);
    }

    void ToggleMenu() { m_menu_open = !m_menu_open; }
    void CloseMenu() { m_menu_open = false; }

    void Draw(Renderer2D &renderer) override
    {
        if (!m_textures_loaded) {
            LoadAssets();
            m_textures_loaded = true;
        }

        // Estética baseada no estado da classe base
        float r = 0.2f, g = 0.2f, b = 0.2f, a = 0.5f;
        float shine_intensity = 0.55f;

        if (m_hovered) {
            r               += 0.1f;
            g               += 0.1f;
            b               += 0.1f;
            a                = 0.8f;
            shine_intensity  = 0.82f;
        }

        renderer.DrawSquircle(geo.x, geo.y, geo.w, geo.h, r, g, b, a, 0.0f, 0.0f, 0.34f);
        renderer.DrawIconShine(geo.x, geo.y, geo.w, geo.h, shine_intensity, 0.0f, 0.0f, 0.34f);

        const float logo_padding = geo.w * 0.15f;
        renderer.DrawTexture(geo.x + logo_padding, geo.y + logo_padding,
                             geo.w - (logo_padding * 2.0f), geo.h - (logo_padding * 2.0f),
                             tex_logo);

        if (m_menu_open) {
            DrawContextMenu(renderer);
        }

        if (m_running) {
            float dot_size = 6.0f;
            float dot_x    = geo.x + (geo.w - dot_size) / 2.0f;
            float dot_y    = geo.y + geo.h + 4.0f;

            renderer.DrawSquircle(dot_x, dot_y, dot_size, dot_size, 1.0f, 1.0f, 1.0f, 0.9f,
                                  dot_size / 2.0f);
        }
    }

    private:
    void DrawContextMenu(Renderer2D &renderer) const
    {
        float menu_w = 180.0f;
        float menu_h = 100.0f;
        float menu_x = geo.x + (geo.w / 2.0f) - (menu_w / 2.0f);
        float menu_y = geo.y - menu_h - 10.0f; // Menu aparece acima do ícone

        // Fundo do menu
        renderer.DrawSquircle(menu_x, menu_y, menu_w, menu_h, 0.1f, 0.1f, 0.12f, 0.98f, 8.0f);

        // Itens (mockup visual por enquanto)
        renderer.DrawRect(menu_x + 5.0f, menu_y + 10.0f, menu_w - 10.0f, 35.0f, 0.2f, 0.2f, 0.25f,
                          0.8f);
        renderer.DrawRect(menu_x + 5.0f, menu_y + 55.0f, menu_w - 10.0f, 35.0f, 0.2f, 0.2f, 0.25f,
                          0.8f);
    }
};
} // namespace unity::shell

#endif
