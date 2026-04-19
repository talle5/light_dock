#version 300 es
precision mediump float;
layout (location = 0) in vec4 vertex;
uniform mat4 pr;
out vec2 f_uv;

void main() {
    gl_Position = pr * vec4(vertex.xy, 0.0, 1.0);
    f_uv = vertex.zw;
}