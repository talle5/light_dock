#pragma once

#include <array>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "../Graphics/GraphicsEngine.h"
#include "../Graphics/Renderer2D.h"
#include "../Graphics/WaylandContext.h"
#include "../Graphics/WindowTracker.h"
#include "../Math/Rect.h"

struct RenderSurfaceRuntime {
    GraphicsEngine engine;
    Renderer2D renderer;
    int width  = 0;
    int height = 0;

    bool Initialize(wl_display *display, wl_surface *target_surface, uint32_t w, uint32_t h)
    {
        width  = w;
        height = h;

        if (!engine.Initialize(display, target_surface, width, height)) {
            return false;
        }

        renderer.InitializeGL();
        renderer.SetSize(width, height);
        return true;
    }

    [[nodiscard]] bool Resize(int w, int h)
    {
        if (width == w && height == h) {
            return false;
        }

        width  = w;
        height = h;
        engine.ResizeWindow(width, height);
        renderer.SetSize(width, height);
        return true;
    }
};

struct ShellRuntime {
    inline static const std::string kLauncherSurfaceId = "launcher";
    inline static const std::string kTopPanelSurfaceId = "top_panel";

    WaylandContext wayland;
    WindowTracker tracker;
    std::array<std::unique_ptr<RenderSurfaceRuntime>,
               static_cast<size_t>(WaylandContext::SurfaceId::Count)>
        surfaces;
    Rect m_dock_geometry;

    static std::optional<std::unique_ptr<ShellRuntime> > Create(uint32_t panel_height,
                                                                uint32_t dock_height)
    {
        auto runtime = std::make_unique<ShellRuntime>();
        runtime->wayland.SetWindowTracker(&runtime->tracker);

        if (!runtime->wayland.Connect()) {
            return std::nullopt;
        }

        // Passando '0' na largura para o Wayland assumir o tamanho do monitor
        runtime->wayland.CreateSurface(WaylandContext::SurfaceId::TopPanel, 0, panel_height,
                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP
                                           | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
                                           | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);

        runtime->wayland.CreateSurface(WaylandContext::SurfaceId::Launcher, 0, dock_height,
                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM
                                           | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT
                                           | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);

        runtime->wayland.Roundtrip();

        if (!runtime->wayland.IsConfigured()) {
            return std::nullopt;
        }

        runtime->tracker.Initialize(runtime->wayland.GetRegistry());

        runtime->CreateSurfaceRuntime(WaylandContext::SurfaceId::TopPanel);
        runtime->CreateSurfaceRuntime(WaylandContext::SurfaceId::Launcher);

        runtime->m_dock_geometry = runtime->MakeDockGeometry();
        return std::make_optional(std::move(runtime));
    }

    RenderSurfaceRuntime &Surface(const WaylandContext::SurfaceId id) const
    { return *surfaces[static_cast<size_t>(id)]; }

    [[nodiscard]] bool SyncDockSurface()
    {
        const auto &state   = wayland.GetSurfaceState(WaylandContext::SurfaceId::Launcher);
        const auto &runtime = surfaces[static_cast<size_t>(WaylandContext::SurfaceId::Launcher)];

        if (!runtime) {
            return false;
        }

        const bool resized = runtime->Resize(state.width, state.height);
        if (resized) {
            m_dock_geometry = MakeDockGeometry();
        }

        return resized;
    }

    [[nodiscard]] Rect DockGeometry() const { return m_dock_geometry; }

    private:

    bool CreateSurfaceRuntime(WaylandContext::SurfaceId id)
    {
        const auto &state = wayland.GetSurfaceState(id);
        auto runtime      = std::make_unique<RenderSurfaceRuntime>();
        if (!runtime->Initialize(wayland.GetDisplay(), state.surface, state.width, state.height)) {
            return false;
        }

        surfaces[static_cast<size_t>(id)] = std::move(runtime);
        return true;
    }

    [[nodiscard]] Rect MakeDockGeometry() const
    {
        const auto &state = wayland.GetSurfaceState(WaylandContext::SurfaceId::Launcher);
        return {0, 0, state.width, state.height};
    }
};
