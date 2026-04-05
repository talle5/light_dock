cmake_minimum_required(VERSION 3.10)
project(unity-shell-wayland)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

find_package(PkgConfig REQUIRED)

pkg_check_modules(WAYLAND REQUIRED wayland-client wayland-egl wayland-cursor)
pkg_check_modules(EGL REQUIRED egl)
pkg_check_modules(GLES REQUIRED glesv2)

#pkg_check_modules(GDK3 REQUIRED gdk-3.0)
#pkg_check_modules(GLIB REQUIRED glib-2.0 gobject-2.0)
#pkg_check_modules(CAIRO REQUIRED cairo)
#pkg_check_modules(SIGC REQUIRED sigc++-2.0)
#pkg_check_modules(AYATANA_INDICATOR REQUIRED ayatana-indicator3-0.4)

add_executable(unity_shell
        main.cpp
        layer-shell/wlr-layer-shell-unstable-v1-protocol.c
        layer-shell/xdg-shell-protocol.c
        layer-shell/wlr-foreign-toplevel-management-v1.c
        Graphics/stb_image.cpp
        Graphics/WaylandContext.h
        Graphics/WindowTracker.h
)

target_include_directories(unity_shell PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/launcher
    ${CMAKE_CURRENT_SOURCE_DIR}/unity-shared
    ${CMAKE_CURRENT_SOURCE_DIR}/UnityCore

    ${WAYLAND_INCLUDE_DIRS}
    ${EGL_INCLUDE_DIRS}
    ${GLES_INCLUDE_DIRS}
#    ${GDK3_INCLUDE_DIRS}
#    ${GLIB_INCLUDE_DIRS}
#    ${AYATANA_INDICATOR_INCLUDE_DIRS}
)

target_link_libraries(unity_shell
    ${WAYLAND_LIBRARIES}
    ${EGL_LIBRARIES}
    ${GLES_LIBRARIES}
#    ${GDK3_LIBRARIES}
#    ${GLIB_LIBRARIES}
#    ${CAIRO_LIBRARIES}
#    ${SIGC_LIBRARIES}
#    ${AYATANA_INDICATOR_LIBRARIES}
    pthread
    dl
)

target_compile_definitions(unity_shell PRIVATE
    GETTEXT_PACKAGE="unity"
    PKGDATADIR="/usr/share/unity"
)

target_compile_options(unity_shell PRIVATE
    -Wno-deprecated-declarations
)
