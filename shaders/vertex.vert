#version 300 es
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;

// As matrizes fundamentais para posição e escala
uniform mat4 pr;
uniform mat4 model;

out vec2 f_uv;

void main() {
    f_uv = uv;
    gl_Position = pr * model * vec4(position, 0.0, 1.0);
}