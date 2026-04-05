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
