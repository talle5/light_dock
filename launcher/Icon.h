#pragma once
#include <string>
#include <utility>
#include "Math/Rect.h"

class Renderer2D;

namespace unity::shell
{

class Icon
{
public:
    nux::Rect geo;
    bool m_hovered = false;
    bool m_running = false;
    unsigned int m_textureId = 0;
    std::string m_tooltipText;
    std::string m_appId;

    explicit Icon(nux::Rect  rect = {0,0,0,0}) : geo(std::move(rect)) {}
    virtual ~Icon() = default;

    virtual void SetPosition(float x, float y) { geo.x = x; geo.y = y; }
    virtual void SetSize(float size) { geo.w = size; geo.h = size; }
    virtual void SetHovered(bool hovered) { m_hovered = hovered; }
    virtual void SetTexture(unsigned int tex) { m_textureId = tex; }

    void SetAppId(const std::string& id) { m_appId = id; }
    [[nodiscard]] std::string_view GetAppId() const { return m_appId; }

    void SetRunning(bool running) { m_running = running; }

    virtual void OnClick() { /* Ação: abrir menu de contexto */ }
    virtual void Draw(Renderer2D& renderer) = 0;
};

} // namespace unity::shell
