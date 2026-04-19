cmake_minimum_required(VERSION 3.10)
project(ligth-dock)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

find_package(PkgConfig REQUIRED)

pkg_check_modules(WAYLAND REQUIRED wayland-client wayland-egl wayland-cursor)
pkg_check_modules(EGL REQUIRED egl)
pkg_check_modules(GLES REQUIRED glesv2)

add_subdirectory(lunasvg)

add_executable(unity_shell
        main.cpp
        layer-shell/wlr-layer-shell-unstable-v1-protocol.c
        layer-shell/xdg-shell-protocol.c
        layer-shell/wlr-foreign-toplevel-management-v1.c
        Graphics/stb_image.cpp
        Graphics/WaylandContext.h
        Graphics/WindowTracker.h
        XdgUtility.h
        Graphics/shaders.h
)

target_include_directories(unity_shell PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/launcher
    ${CMAKE_CURRENT_SOURCE_DIR}/unity-shared
    ${CMAKE_CURRENT_SOURCE_DIR}/UnityCore

    ${WAYLAND_INCLUDE_DIRS}
    ${EGL_INCLUDE_DIRS}
    ${GLES_INCLUDE_DIRS}
)

target_link_libraries(unity_shell
    ${WAYLAND_LIBRARIES}
    ${EGL_LIBRARIES}
    ${GLES_LIBRARIES}
    lunasvg
)

target_compile_definitions(unity_shell PRIVATE
    GETTEXT_PACKAGE="unity"
    PKGDATADIR="/usr/share/unity"
)

target_compile_options(unity_shell PRIVATE
    -Wno-deprecated-declarations
)
