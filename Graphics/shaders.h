//
// Created by talles on 18/04/2026.
//

#ifndef LIGTH_DOCK_SHADERS_H
#define LIGTH_DOCK_SHADERS_H
inline const char *basic_vert = R"glsl(
#version 300 es
layout(location=0) in vec2 p;
layout(location=1) in vec2 uv;
uniform mat4 pr;
uniform mat4 model;

out vec2 f_uv;

void main() {
    vec4 pos = pr * model * vec4(p, 0.0, 1.0);
    pos.w = 1.0 - (pos.z * 0.0025);
    gl_Position = pos;
    f_uv = uv;
}
)glsl";

inline const char *icon_shine_frag = R"glsl(
#version 300 es
precision mediump float;

in vec2 f_uv;
uniform vec2 dimensions;
uniform float radius_ratio;
uniform float intensity;

out vec4 f;

void main() {
    // --- 1. SHAPE ---
    vec2 size = max(dimensions, vec2(1.0));
    vec2 center = size * 0.5;
    vec2 p = (f_uv * size) - center;

    float radius = min(size.x, size.y) * clamp(radius_ratio, 0.05, 0.5);
    vec2 half_extents = max(center - vec2(radius), vec2(0.0));
    vec2 q = abs(p) - half_extents;

    float d = length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - radius;

    // anti-alias
    float aa = fwidth(d);
    float alpha_mask = 1.0 - smoothstep(0.0, aa, d);

    if (alpha_mask <= 0.0) discard;

    // --- 2. UV normalizado centrado ---
    vec2 uv = f_uv;
    vec2 centered = uv * 2.0 - 1.0;

    // --- 3. BASE LIGHT---
    vec2 light_source = vec2(0.5, 1.0);
    vec2 offset = uv - light_source;

    offset.x *= 1.4; // leve ajuste lateral

    float dist = length(offset);

    float volume_light = smoothstep(0.9, 0.0, dist);
    volume_light = pow(volume_light, 1.8); // mais suave

    // --- 4. HIGHLIGHT DIRECIONAL---
    vec2 dir = normalize(vec2(0.8, -0.6));
    float spec = dot(centered, dir);

    spec = smoothstep(0.2, 0.8, spec);
    spec = pow(spec, 4.0); // brilho concentrado

    // --- 5. SOMBRA LEVE---
    float shadow = smoothstep(0.2, 1.2, centered.y + 0.2);

    // --- 6. COMBINAÇÃO FINAL ---
    float light = volume_light * 0.8 + spec * 0.6;
    light *= shadow;

    float final_alpha = light * intensity * alpha_mask;

    f = vec4(vec3(1.0), final_alpha);
})glsl";

inline const char *rect_frag     = R"glsl(
#version 300 es
precision mediump float;
uniform vec4 color;
out vec4 f;
void main() {
    f = color;
}
)glsl";
inline const char *squircle_frag = R"glsl(
#version 300 es
precision mediump float;
in vec2 f_uv;
uniform vec2 dimensions;
uniform float radius_ratio;
uniform vec4 color;
out vec4 f;

void main() {
    vec2 size = max(dimensions, vec2(1.0));
    vec2 center = size * 0.5;
    vec2 p = (f_uv * size) - center;

    float radius = min(size.x, size.y) * clamp(radius_ratio, 0.05, 0.5);
    vec2 half_extents = max(center - vec2(radius), vec2(0.0));
    vec2 q = abs(p) - half_extents;
    float distance = length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - radius;
    float alpha = 1.0 - smoothstep(0.0, 1.5, distance);

    if (alpha <= 0.0) discard;
    f = vec4(color.rgb, color.a * alpha);
}

)glsl";
inline const char *text_frag     = R"glsl(
#version 300 es
precision mediump float;

in vec2 f_uv;
uniform sampler2D tex;
uniform vec4 textColor;

out vec4 f;

void main() {
    float alpha = texture(tex, f_uv).r;
    f = vec4(textColor.rgb, textColor.a * alpha);
}
)glsl";
inline const char *text_vert     = R"glsl(
#version 300 es
precision mediump float;
layout (location = 0) in vec4 vertex;
uniform mat4 pr;
out vec2 f_uv;

void main() {
    gl_Position = pr * vec4(vertex.xy, 0.0, 1.0);
    f_uv = vertex.zw;
}
)glsl";
inline const char *texture_frag  = R"glsl(
#version 300 es
precision mediump float;

in vec2 f_uv;
uniform sampler2D tex;

out vec4 f;

void main() {
    vec4 tex_color = texture(tex, f_uv);

    if (tex_color.a <= 0.05) {
        discard;
    }

    f = tex_color;
}
)glsl";
inline const char *texture_vert  = R"glsl(
#version 300 es
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;
uniform mat4 pr;
uniform mat4 model;

out vec2 f_uv;

void main() {
    f_uv = uv;
    gl_Position = pr * model * vec4(position, 0.0, 1.0);
}
)glsl";

#endif // LIGTH_DOCK_SHADERS_H
