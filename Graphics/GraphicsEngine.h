#pragma once

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <iostream>

namespace nux
{
    class GraphicsEngine
    {
        inline static EGLDisplay shared_egl_display = EGL_NO_DISPLAY;
        inline static EGLContext shared_egl_context = EGL_NO_CONTEXT;
        inline static EGLConfig shared_egl_config = nullptr;
        inline static int shared_instance_count = 0;

        EGLSurface egl_surface = EGL_NO_SURFACE;
        wl_egl_window* egl_window = nullptr;

        int width = 0;
        int height = 0;

    public:
        GraphicsEngine() = default;
        GraphicsEngine(const GraphicsEngine&) = delete;
        GraphicsEngine& operator=(const GraphicsEngine&) = delete;

        static GraphicsEngine& GetInstance()
        {
            static GraphicsEngine instance;
            return instance;
        }

        ~GraphicsEngine()
        {
            Shutdown();
        }

        // --- GETTERS ---
        [[nodiscard]] int GetWidth() const { return width; }
        [[nodiscard]] int GetHeight() const { return height; }

        // --- INICIALIZAÇÃO ---
        bool Initialize(wl_display* display, wl_surface* surface, int w, int h)
        {
            width = w;
            height = h;

            if (!InitSharedDisplay(display)) return false;
            if (!ChooseSharedConfig()) return false;
            if (!CreateSharedContext()) return false;
            if (!CreateSurface(surface)) return false;

            if (!eglMakeCurrent(shared_egl_display, egl_surface, egl_surface, shared_egl_context))
            {
                std::cerr << "EGL: falha eglMakeCurrent\n";
                return false;
            }

            ++shared_instance_count;
            // Define a lente inicial
            glViewport(0, 0, width, height);
            return true;
        }

        // --- CICLO DE VIDA ---
        void ResizeWindow(int w, int h)
        {
            width = w;
            height = h;

            if (!egl_window) return;
            wl_egl_window_resize(egl_window, width, height, 0, 0);
            eglMakeCurrent(shared_egl_display, egl_surface, egl_surface, shared_egl_context);
            glViewport(0, 0, width, height);
        }

        void BeginFrame() const
        {
            eglMakeCurrent(shared_egl_display, egl_surface, egl_surface, shared_egl_context);
        }

        void ClearScreen(float r, float g, float b, float a = 1.0f) const
        {
            glClearColor(r, g, b, a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        void EndFrame()
        {
            eglSwapBuffers(shared_egl_display, egl_surface);
        }

        void Shutdown()
        {
            if (shared_egl_display == EGL_NO_DISPLAY) return;

            eglMakeCurrent(shared_egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

            if (egl_surface != EGL_NO_SURFACE)
            {
                eglDestroySurface(shared_egl_display, egl_surface);
                egl_surface = EGL_NO_SURFACE;
            }
            if (egl_window)
            {
                wl_egl_window_destroy(egl_window);
                egl_window = nullptr;
            }

            if (shared_instance_count > 0)
            {
                --shared_instance_count;
            }

            if (shared_instance_count == 0)
            {
                if (shared_egl_context != EGL_NO_CONTEXT)
                {
                    eglDestroyContext(shared_egl_display, shared_egl_context);
                    shared_egl_context = EGL_NO_CONTEXT;
                }

                eglTerminate(shared_egl_display);
                shared_egl_display = EGL_NO_DISPLAY;
                shared_egl_config = nullptr;
            }
        }

    private:
        bool InitSharedDisplay(wl_display* display)
        {
            if (shared_egl_display != EGL_NO_DISPLAY)
            {
                return true;
            }

            shared_egl_display = eglGetDisplay(display);

            if (shared_egl_display == EGL_NO_DISPLAY)
            {
                std::cerr << "EGL: falha eglGetDisplay\n";
                return false;
            }

            EGLint major, minor;

            if (!eglInitialize(shared_egl_display, &major, &minor))
            {
                std::cerr << "EGL: falha eglInitialize\n";
                return false;
            }

            return true;
        }

        bool ChooseSharedConfig()
        {
            if (shared_egl_config != nullptr)
            {
                return true;
            }

            const EGLint attribs[] =
            {
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
                EGL_NONE
            };

            EGLint num_configs = 0;

            if (!eglChooseConfig(shared_egl_display, attribs, &shared_egl_config, 1, &num_configs) || num_configs == 0)
            {
                std::cerr << "EGL: falha eglChooseConfig\n";
                return false;
            }

            return true;
        }

        bool CreateSharedContext()
        {
            if (shared_egl_context != EGL_NO_CONTEXT)
            {
                return true;
            }

            const EGLint context_attribs[] =
            {
                EGL_CONTEXT_CLIENT_VERSION, 3,
                EGL_NONE
            };

            shared_egl_context = eglCreateContext(
                shared_egl_display,
                shared_egl_config,
                EGL_NO_CONTEXT,
                context_attribs
            );

            if (shared_egl_context == EGL_NO_CONTEXT)
            {
                std::cerr << "EGL: falha eglCreateContext\n";
                return false;
            }

            return true;
        }

        bool CreateSurface(wl_surface* surface)
        {
            egl_window = wl_egl_window_create(surface, width, height);

            if (!egl_window)
            {
                std::cerr << "Wayland: falha wl_egl_window_create\n";
                return false;
            }

            egl_surface = eglCreateWindowSurface(
                shared_egl_display,
                shared_egl_config,
                reinterpret_cast<EGLNativeWindowType>(egl_window),
                nullptr
            );

            if (egl_surface == EGL_NO_SURFACE)
            {
                std::cerr << "EGL: falha eglCreateWindowSurface\n";
                return false;
            }

            return true;
        }
    };
}
