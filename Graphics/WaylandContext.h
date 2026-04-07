#pragma once

#include <array>
#include <cstring>
#include <functional>
#include <linux/input-event-codes.h>
#include <wayland-client.h>
#include "WindowTracker.h"

extern "C" {
#include "../layer-shell/wlr-layer-shell-unstable-v1-protocol.h"
}

class WaylandContext
{
public:
    enum class SurfaceId {
        TopPanel,
        Launcher,
        Count
    };

    struct SurfaceState {
        wl_surface* surface = nullptr;
        zwlr_layer_surface_v1* layer_surface = nullptr;
        bool configured = false;
        int width = 0;
        int height = 0;
    };

    struct Callbacks {
        std::function<void(double, double)> move;
        std::function<void(double, double, int)> click;
        std::function<void()> leave;
    };

private:
    std::array<SurfaceState, static_cast<size_t>(SurfaceId::Count)> m_surfaces{};

    wl_display* m_display = nullptr;
    wl_registry* m_registry = nullptr;
    wl_compositor* m_compositor = nullptr;
    zwlr_layer_shell_v1* m_layer_shell = nullptr;
    wl_seat* m_seat = nullptr;
    wl_pointer* m_pointer = nullptr;

    double m_mouse_x = 0.0;
    double m_mouse_y = 0.0;

    Callbacks m_callbacks;
    WindowTracker* m_window_tracker = nullptr;

    // ---------------- POINTER HANDLERS ----------------

    static void pointer_handle_enter(void* data, wl_pointer*, uint32_t, wl_surface*, wl_fixed_t sx, wl_fixed_t sy)
    {
        auto* ctx = static_cast<WaylandContext*>(data);
        ctx->m_mouse_x = wl_fixed_to_double(sx);
        ctx->m_mouse_y = wl_fixed_to_double(sy);
        if (ctx->m_callbacks.move)
            ctx->m_callbacks.move(ctx->m_mouse_x, ctx->m_mouse_y);
    }

    static void pointer_handle_motion(void* data, wl_pointer*, uint32_t, wl_fixed_t sx, wl_fixed_t sy)
    {
        auto* ctx = static_cast<WaylandContext*>(data);
        ctx->m_mouse_x = wl_fixed_to_double(sx);
        ctx->m_mouse_y = wl_fixed_to_double(sy);

        if (ctx->m_callbacks.move)
            ctx->m_callbacks.move(ctx->m_mouse_x, ctx->m_mouse_y);
    }

    static void pointer_handle_button(void* data, wl_pointer*, uint32_t, uint32_t, uint32_t button, uint32_t state)
    {
        auto* ctx = static_cast<WaylandContext*>(data);
        if (state == WL_POINTER_BUTTON_STATE_PRESSED && ctx->m_callbacks.click) {
            int b = (button == BTN_LEFT) ? 1 : (button == BTN_RIGHT ? 3 : 0);
            ctx->m_callbacks.click(ctx->m_mouse_x, ctx->m_mouse_y, b);
        }
    }

    static void pointer_handle_leave(void* data, wl_pointer*, uint32_t, wl_surface*)
    {
        auto *ctx = static_cast<WaylandContext *>(data);
        if (ctx->m_callbacks.leave)
            ctx->m_callbacks.leave();
    }

    static void pointer_handle_axis(void*, wl_pointer*, uint32_t, uint32_t, wl_fixed_t) {}
    static void pointer_handle_frame(void*, wl_pointer*) {}
    static void pointer_handle_axis_source(void*, wl_pointer*, uint32_t) {}
    static void pointer_handle_axis_stop(void*, wl_pointer*, uint32_t, uint32_t) {}
    static void pointer_handle_axis_discrete(void*, wl_pointer*, uint32_t, int32_t) {}

    static constexpr wl_pointer_listener pointer_listener = {
        .enter = pointer_handle_enter,
        .leave = pointer_handle_leave,
        .motion = pointer_handle_motion,
        .button = pointer_handle_button,
        .axis = pointer_handle_axis,
        .frame = pointer_handle_frame,
        .axis_source = pointer_handle_axis_source,
        .axis_stop = pointer_handle_axis_stop,
        .axis_discrete = pointer_handle_axis_discrete
    };

    // ---------------- REGISTRY ----------------

    static void registry_handle_global(void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
    {
        auto* ctx = static_cast<WaylandContext*>(data);
        if (strcmp(interface, wl_compositor_interface.name) == 0)
            ctx->m_compositor = static_cast<wl_compositor*>(wl_registry_bind(registry, name, &wl_compositor_interface, 4));
        else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0)
            ctx->m_layer_shell = static_cast<zwlr_layer_shell_v1*>(wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, 1));
        else if (strcmp(interface, wl_seat_interface.name) == 0) {
            ctx->m_seat = static_cast<wl_seat*>(wl_registry_bind(registry, name, &wl_seat_interface, 1));
            ctx->m_pointer = wl_seat_get_pointer(ctx->m_seat);
            wl_pointer_add_listener(ctx->m_pointer, &pointer_listener, ctx);
        }
        if (ctx->m_window_tracker)
            ctx->m_window_tracker->HandleRegistryGlobal(registry, name, interface, version);
    }

    static void layer_configure(void* data, zwlr_layer_surface_v1* surface, uint32_t serial, uint32_t w, uint32_t h)
    {
        auto* ctx = static_cast<WaylandContext*>(data);
        for (auto& s : ctx->m_surfaces) {
            if (s.layer_surface == surface) {
                s.width = static_cast<int>(w);
                s.height = static_cast<int>(h);
                s.configured = true;
                break;
            }
        }
        zwlr_layer_surface_v1_ack_configure(surface, serial);
    }

    static constexpr zwlr_layer_surface_v1_listener layer_listener = { .configure = layer_configure };

public:
    bool Connect()
    {
        m_display = wl_display_connect(nullptr);
        if (!m_display) return false;
        m_registry = wl_display_get_registry(m_display);
        static constexpr wl_registry_listener reg = { .global = registry_handle_global };
        wl_registry_add_listener(m_registry, &reg, this);
        wl_display_roundtrip(m_display);
        return m_compositor && m_layer_shell;
    }

    void SetWindowTracker(WindowTracker* t) { m_window_tracker = t; }
    void SetCallbacks(const Callbacks& cb) { m_callbacks = cb; }

    wl_surface* CreateSurface(SurfaceId id, uint32_t w, uint32_t h, uint32_t anchor)
    {
        auto& s = m_surfaces[static_cast<size_t>(id)];
        s.surface = wl_compositor_create_surface(m_compositor);
        s.layer_surface = zwlr_layer_shell_v1_get_layer_surface(m_layer_shell, s.surface, nullptr, ZWLR_LAYER_SHELL_V1_LAYER_TOP, "unity");
        zwlr_layer_surface_v1_set_size(s.layer_surface, w, h);
        zwlr_layer_surface_v1_set_anchor(s.layer_surface, anchor);

        if (id == SurfaceId::Launcher)
            zwlr_layer_surface_v1_set_exclusive_zone(s.layer_surface, h);

        zwlr_layer_surface_v1_add_listener(s.layer_surface, &layer_listener, this);
        wl_surface_commit(s.surface);
        return s.surface;
    }

    [[nodiscard]] const SurfaceState& GetSurfaceState(SurfaceId id) const { return m_surfaces[static_cast<size_t>(id)]; }

    [[nodiscard]] wl_display* GetDisplay() const { return m_display; }
    [[nodiscard]] wl_registry* GetRegistry() const { return m_registry; }
    [[nodiscard]] wl_seat* GetSeat() const { return m_seat; }
    [[nodiscard]] bool Dispatch() const { return wl_display_dispatch(m_display) != -1; }
    [[nodiscard]] bool Roundtrip() const { return wl_display_roundtrip(m_display) != -1; }
    [[nodiscard]] bool IsConfigured() const {
        for (const auto& s : m_surfaces) if (s.layer_surface && !s.configured) return false;
        return true;
    }

    ~WaylandContext()
    {
        for (const auto & s : m_surfaces) {
            if (s.layer_surface) zwlr_layer_surface_v1_destroy(s.layer_surface);
            if (s.surface) wl_surface_destroy(s.surface);
        }
        if (m_pointer) wl_pointer_destroy(m_pointer);
        if (m_seat) wl_seat_destroy(m_seat);
        if (m_layer_shell) zwlr_layer_shell_v1_destroy(m_layer_shell);
        if (m_compositor) wl_compositor_destroy(m_compositor);
        if (m_registry) wl_registry_destroy(m_registry);
        if (m_display) wl_display_disconnect(m_display);
    }
};
