#version 300 es
precision mediump float;

in vec2 f_uv;
uniform sampler2D tex;
uniform vec4 textColor; // A cor que queremos dar à letra!

out vec4 f;

void main() {
    // Lemos apenas o canal VERMELHO (r) da textura invisível para saber a "força" da letra
    float alpha = texture(tex, f_uv).r;

    // Multiplicamos a cor que escolhemos (ex: Branco) pelo formato da letra
    f = vec4(textColor.rgb, textColor.a * alpha);
}