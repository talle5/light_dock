#version 300 es
layout(location=0) in vec2 p;
layout(location=1) in vec2 uv;
uniform mat4 pr;
uniform mat4 model;

out vec2 f_uv; // <-- Ele envia isto!

void main() {
    vec4 pos = pr * model * vec4(p, 0.0, 1.0);
    pos.w = 1.0 - (pos.z * 0.0025);
    gl_Position = pos;
    f_uv = uv;     // <-- Ele preenche isto!
}