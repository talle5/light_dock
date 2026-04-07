#pragma once

#include "ShellRuntime.h"
#include "Docky.h"
#include <chrono>
#include <functional>
#include <memory>
#include <optional>

class UnityShell
{
    static constexpr auto kDockWidth = 24;
    static constexpr auto kDockHeight = unity::shell::Docky::RecommendedSurfaceHeight();
    static constexpr auto kAutoHideDelay = std::chrono::milliseconds(250);

    std::unique_ptr<ShellRuntime> m_runtime;
    unity::shell::Docky m_docky;
    bool m_hide_pending = false;
    std::chrono::steady_clock::time_point m_hide_deadline{};
    bool m_launcher_visible = true;

public:
    static std::optional<std::unique_ptr<UnityShell>> Create()
    {
        auto runtime = ShellRuntime::Create(kDockWidth, kDockHeight);
        if (!runtime) return std::nullopt;

        auto shell = std::unique_ptr<UnityShell>(new UnityShell(std::move(runtime.value())));
        shell->BindCallbacks();
        return std::make_optional(std::move(shell));
    }

    void Run()
    {
        do
        {
            Update();
            OnFrameReady();
        }while (m_runtime->wayland.Dispatch());
    }

private:
    explicit UnityShell(std::unique_ptr<ShellRuntime> runtime)
        : m_runtime(std::move(runtime))
    {
        m_docky.SetGeometry(m_runtime->DockGeometry());
    }

    void BindCallbacks()
    {
        m_runtime->wayland.SetCallbacks({
            .move = [this](double x, double y) {
                ShowDock();
                m_docky.HandlePointerMotion(x, y);
            },
            .click = [this](double x, double y, int button) {
                ShowDock();
                m_docky.HandlePointerClick(x, y, button);
            },
            .leave = [this]() {
                m_docky.HandlePointerLeave();
                ScheduleHideDock();
            }
        });
    }

    void Update()
    {
        m_docky.UpdateOpenApps(m_runtime->tracker.GetOpenApps());

        if (!m_runtime->tracker.HasActiveVisibleWindow())
        {
            ShowDock();
        }

        if (m_hide_pending && std::chrono::steady_clock::now() >= m_hide_deadline)
        {
            HideDock();
        }
    }

    void OnFrameReady()
    {
        if (m_runtime->SyncDockSurface())
        {
            m_docky.SetGeometry(m_runtime->DockGeometry());
        }

        auto& launcher_runtime = m_runtime->Surface(ShellRuntime::kLauncherSurfaceId);
        launcher_runtime.engine.BeginFrame();
        launcher_runtime.engine.ClearScreen(0.0f, 0.0f, 0.0f, 0.0f);

        if (m_launcher_visible)
        {
            m_docky.ComputeLayoutAndDraw(launcher_runtime.renderer);
        }

        launcher_runtime.engine.EndFrame();
    }

    void ShowDock()
    {
        m_hide_pending = false;
        m_launcher_visible = true;
    }

    void HideDock()
    {
        m_hide_pending = false;
        if (m_runtime->tracker.HasActiveVisibleWindow())
        {
            m_launcher_visible = false;
        }
    }

    void ScheduleHideDock()
    {
        m_hide_pending = true;
        m_hide_deadline = std::chrono::steady_clock::now() + kAutoHideDelay;
    }
};
