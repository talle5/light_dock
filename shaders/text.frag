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