#ifndef UNITY_SHELL_WAYLAND_RENDER2D_H
#define UNITY_SHELL_WAYLAND_RENDER2D_H

#include "shader.h"
#include "shaders.h"
#include <GLES3/gl3.h>
#include <array>
#include <cmath>

class Renderer2D
{
    Shader m_texShader;
    Shader m_rectShader;
    Shader m_squircleShader;
    Shader m_shineShader;

    GLuint m_vao = 0;
    GLuint m_vbo = 0;

    std::array<float, 16> ortomatrix{};

    public:
    Renderer2D() = default;

    void InitializeGL()
    {
        constexpr std::array<float, 16> quad = {0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0};

        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, 64, quad.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                              reinterpret_cast<void *>(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        InitShaders();
    }

    void SetOrthoMatriz(const std::array<float, 16> &mat) { ortomatrix = mat; }

    float *GetOrthoMatriz() { return ortomatrix.data(); }

    private:
    void InitShaders()
    {
        m_texShader.Compile(basic_vert, texture_frag);
        m_rectShader.Compile(basic_vert,rect_frag);
        m_squircleShader.Compile(basic_vert,squircle_frag);
        m_shineShader.Compile(basic_vert,icon_shine_frag);
    }

    public:
    void SetSize(int w, int h) { ortomatrix = MakeOrthoMatrix(w, h); }

    static constexpr std::array<float, 16> MakeOrthoMatrix(float width, float height)
    {
        constexpr float L = 0.0f;
        const float R     = width;
        constexpr float T = 0.0f;
        const float B     = height;
        std::array<float, 16> m{};
        m[0]  = 2.0f / (R - L);
        m[5]  = -2.0f / (B - T); // <--- O valor negativo que inverte o Y!
        m[10] = -0.001f;
        m[12] = -1.0f;
        m[13] = 1.0f; // <--- O valor que ajusta o topo
        m[15] = 1;
        return m;
    }

    static void MakeModel(float *m, float x, float y, float w, float h, float z = 0.0f,
                          float rotX = 0.0f)
    {
        const float cx = cos(rotX);
        const float sx = sin(rotX);
        m[0]           = w;
        m[4]           = 0;
        m[8]           = 0;
        m[12]          = x;
        m[1]           = 0;
        m[5]           = h * cx;
        m[9]           = 0;
        m[13]          = y + h - (h * cx);
        m[2]           = 0;
        m[6]           = h * sx;
        m[10]          = cx;
        m[14]          = z - (h * sx);
        m[3]           = 0;
        m[7]           = 0;
        m[11]          = 0;
        m[15]          = 1;
    }

    void DrawTexture(float x, float y, float w, float h, GLuint texture, float z = 0.0f,
                     float rotX = 0.0f)
    {
        float model[16];
        MakeModel(model, x, y, w, h, z, rotX);
        m_texShader.Use();
        m_texShader.SetMatrix4("pr", ortomatrix.data());
        m_texShader.SetMatrix4("model", model);
        m_texShader.SetInteger("tex", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(m_vao);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void DrawRect(float x, float y, float w, float h, float r, float g, float b, float a,
                  float z = 0.0f, float rotX = 0.0f)
    {
        float model[16];
        MakeModel(model, x, y, w, h, z, rotX);
        m_rectShader.Use();
        m_rectShader.SetMatrix4("pr", ortomatrix.data());
        m_rectShader.SetMatrix4("model", model);
        m_rectShader.SetVector4("color", r, g, b, a);
        glBindVertexArray(m_vao);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void DrawSquircle(float x, float y, float w, float h, float r, float g, float b, float a,
                      float z = 0.0f, float rotX = 0.0f, float radius_ratio = 0.28f)
    {
        float model[16];
        MakeModel(model, x, y, w, h, z, rotX);
        m_squircleShader.Use();
        m_squircleShader.SetMatrix4("pr", ortomatrix.data());
        m_squircleShader.SetMatrix4("model", model);
        m_squircleShader.SetVector2("dimensions", w, h);
        m_squircleShader.SetFloat("radius_ratio", radius_ratio);
        m_squircleShader.SetVector4("color", r, g, b, a);
        glBindVertexArray(m_vao);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void DrawIconShine(float x, float y, float w, float h, float intensity = 1.0f, float z = 0.0f,
                       float rotX = 0.0f, float radius_ratio = 0.28f)
    {
        float model[16];
        MakeModel(model, x, y, w, h, z, rotX);
        m_shineShader.Use();
        m_shineShader.SetMatrix4("pr", ortomatrix.data());
        m_shineShader.SetMatrix4("model", model);
        m_shineShader.SetVector2("dimensions", w, h);
        m_shineShader.SetFloat("radius_ratio", radius_ratio);
        m_shineShader.SetFloat("intensity", intensity);
        glBindVertexArray(m_vao);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
};
#endif
