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
}