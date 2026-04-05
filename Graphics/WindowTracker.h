//
// Created by talles on 25/03/2026.
//

#ifndef UNITY_SHELL_WAYLAND_WINDOWTRACKER_H
#define UNITY_SHELL_WAYLAND_WINDOWTRACKER_H

#include <algorithm>
#include <iostream>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

#include <wayland-client.h>

#include "layer-shell/wlr-foreign-toplevel-management-v1.h"

class WindowTracker
{
public:
    struct OpenApp
    {
        zwlr_foreign_toplevel_handle_v1* handle = nullptr;
        std::string title;
        std::string app_id;
        bool minimized = false;
        bool activated = false;
    };

private:
    zwlr_foreign_toplevel_manager_v1* m_manager = nullptr;
    std::unordered_map<zwlr_foreign_toplevel_handle_v1*, OpenApp> m_open_apps;

    static void manager_handle_toplevel(void* data,
                                        zwlr_foreign_toplevel_manager_v1* manager,
                                        zwlr_foreign_toplevel_handle_v1* toplevel)
    {
        static_cast<WindowTracker*>(data)->OnToplevelCreated(toplevel);
    }

    static void manager_handle_finished(void* data,
                                        zwlr_foreign_toplevel_manager_v1* manager)
    {
        static_cast<WindowTracker*>(data)->OnManagerFinished();
    }

    static void handle_title_c(void* data,
                               zwlr_foreign_toplevel_handle_v1* toplevel,
                               const char* title)
    {
        static_cast<WindowTracker*>(data)->OnTitleChanged(toplevel, title);
    }

    static void handle_app_id_c(void* data,
                                zwlr_foreign_toplevel_handle_v1* toplevel,
                                const char* app_id)
    {
        static_cast<WindowTracker*>(data)->OnAppIdChanged(toplevel, app_id);
    }

    static void handle_output_enter_c(void* data,
                                      zwlr_foreign_toplevel_handle_v1* toplevel,
                                      wl_output* output)
    {
    }

    static void handle_output_leave_c(void* data,
                                      zwlr_foreign_toplevel_handle_v1* toplevel,
                                      wl_output* output)
    {
    }

    static void handle_state_c(void* data,
                               zwlr_foreign_toplevel_handle_v1* toplevel,
                               wl_array* state)
    {
        static_cast<WindowTracker*>(data)->OnStateChanged(toplevel, state);
    }

    static void handle_done_c(void* data,
                              zwlr_foreign_toplevel_handle_v1* toplevel)
    {
    }

    static void handle_closed_c(void* data,
                                zwlr_foreign_toplevel_handle_v1* toplevel)
    {
        static_cast<WindowTracker*>(data)->OnClosed(toplevel);
    }

    static void handle_parent_c(void* data,
                                zwlr_foreign_toplevel_handle_v1* toplevel,
                                zwlr_foreign_toplevel_handle_v1* parent)
    {
    }

public:
    ~WindowTracker()
    {
        for (const auto& handle : m_open_apps | std::views::keys)
        {
            if (handle)
            {
                zwlr_foreign_toplevel_handle_v1_destroy(handle);
            }
        }

        if (m_manager)
        {
            zwlr_foreign_toplevel_manager_v1_destroy(m_manager);
        }
    }

    void Initialize(wl_registry* registry)
    {
        (void)registry;
    }

    void OnTitleChanged(zwlr_foreign_toplevel_handle_v1* toplevel, const char* title)
    {
        auto& app = m_open_apps[toplevel];
        app.handle = toplevel;
        app.title = title ? title : "";
        std::cout << "[WindowTracker] title updated: handle=" << toplevel
                  << " title=\"" << app.title << "\"\n";
    }

    void OnAppIdChanged(zwlr_foreign_toplevel_handle_v1* toplevel, const char* app_id)
    {
        auto& app = m_open_apps[toplevel];
        app.handle = toplevel;
        app.app_id = app_id ? app_id : "";
        std::cout << "[WindowTracker] app_id updated: handle=" << toplevel
                  << " app_id=\"" << app.app_id << "\"\n";
    }

    [[nodiscard]] std::vector<OpenApp> GetOpenApps() const
    {
        std::vector<OpenApp> apps;
        apps.reserve(m_open_apps.size());

        for (const auto& app : m_open_apps | std::views::values)
        {
            apps.push_back(app);
        }

        std::ranges::sort(apps, [](const OpenApp& lhs, const OpenApp& rhs) {
            if (lhs.title == rhs.title)
            {
                return lhs.app_id < rhs.app_id;
            }

            return lhs.title < rhs.title;
        });

        return apps;
    }

    [[nodiscard]] bool ActivateApp(const std::string& app_id, wl_seat* seat) const
    {
        if (!seat || app_id.empty())
        {
            return false;
        }

        for (const auto& app : m_open_apps | std::views::values)
        {
            if (app.app_id != app_id || !app.handle)
            {
                continue;
            }

            zwlr_foreign_toplevel_handle_v1_activate(app.handle, seat);
            return true;
        }

        return false;
    }

    [[nodiscard]] bool ToggleMinimizeOrActivateApp(const std::string& app_id, wl_seat* seat) const
    {
        if (!seat || app_id.empty())
        {
            return false;
        }

        for (const auto& app : m_open_apps | std::views::values)
        {
            if (app.app_id != app_id || !app.handle)
            {
                continue;
            }

            if (app.minimized)
            {
                zwlr_foreign_toplevel_handle_v1_unset_minimized(app.handle);
                zwlr_foreign_toplevel_handle_v1_activate(app.handle, seat);
                return true;
            }

            if (app.activated)
            {
                zwlr_foreign_toplevel_handle_v1_set_minimized(app.handle);
                return true;
            }

            zwlr_foreign_toplevel_handle_v1_activate(app.handle, seat);
            return true;
        }

        return false;
    }

    [[nodiscard]] bool HasActiveVisibleWindow() const
    {
        return std::ranges::any_of(m_open_apps | std::views::values, [](const OpenApp& app) {
            return app.activated && !app.minimized;
        });
    }

    void HandleRegistryGlobal(wl_registry* registry,
                              uint32_t name,
                              const char* interface,
                              uint32_t version)
    {
        if (!interface || m_manager)
        {
            return;
        }

        if (std::string(interface) != zwlr_foreign_toplevel_manager_v1_interface.name)
        {
            return;
        }

        const uint32_t bind_version = std::min<uint32_t>(version, 3U);
        m_manager = static_cast<zwlr_foreign_toplevel_manager_v1*>(
            wl_registry_bind(registry, name, &zwlr_foreign_toplevel_manager_v1_interface, bind_version));
        std::cout << "[WindowTracker] foreign toplevel manager bound\n";

        static const zwlr_foreign_toplevel_manager_v1_listener manager_listener = {
            .toplevel=manager_handle_toplevel,
            .finished=manager_handle_finished
        };

        zwlr_foreign_toplevel_manager_v1_add_listener(m_manager, &manager_listener, this);
    }

private:
    void OnStateChanged(zwlr_foreign_toplevel_handle_v1* toplevel, wl_array* state)
    {
        const auto it = m_open_apps.find(toplevel);
        if (it == m_open_apps.end() || !state)
        {
            return;
        }

        bool minimized = false;
        bool activated = false;

        const auto* begin = static_cast<const uint32_t*>(state->data);
        const auto* end = begin + (state->size / sizeof(uint32_t));
        for (auto* current = begin; current != end; ++current)
        {
            if (*current == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MINIMIZED)
            {
                minimized = true;
            }
            else if (*current == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_ACTIVATED)
            {
                activated = true;
            }
        }

        it->second.minimized = minimized;
        it->second.activated = activated;
    }

    void OnToplevelCreated(zwlr_foreign_toplevel_handle_v1* toplevel)
    {
        if (!toplevel)
        {
            return;
        }

        m_open_apps[toplevel] = OpenApp{toplevel, "", ""};
        std::cout << "[WindowTracker] window created: handle=" << toplevel << '\n';

        static const zwlr_foreign_toplevel_handle_v1_listener handle_listener = {
            .title=handle_title_c,
            .app_id=handle_app_id_c,
            .output_enter=handle_output_enter_c,
            .output_leave=handle_output_leave_c,
            .state=handle_state_c,
            .done=handle_done_c,
            .closed=handle_closed_c,
            .parent=handle_parent_c
        };

        zwlr_foreign_toplevel_handle_v1_add_listener(toplevel, &handle_listener, this);
    }

    void OnManagerFinished()
    {
        std::cout << "[WindowTracker] foreign toplevel manager finished\n";
        m_manager = nullptr;
    }

    void OnClosed(zwlr_foreign_toplevel_handle_v1* toplevel)
    {
        const auto it = m_open_apps.find(toplevel);
        if (it == m_open_apps.end())
        {
            return;
        }

        std::cout << "[WindowTracker] window closed: handle=" << toplevel
                  << " title=\"" << it->second.title
                  << "\" app_id=\"" << it->second.app_id << "\"\n";
        zwlr_foreign_toplevel_handle_v1_destroy(toplevel);
        m_open_apps.erase(it);
    }
};
#endif //UNITY_SHELL_WAYLAND_WINDOWTRACKER_H
