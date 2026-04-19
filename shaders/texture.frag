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