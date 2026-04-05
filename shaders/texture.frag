#version 300 es
precision mediump float;

in vec2 f_uv;
uniform sampler2D tex;

out vec4 f;

void main() {
    // Lê o pixel da imagem
    vec4 tex_color = texture(tex, f_uv);

    // Se for transparente, corta para não bugar a sombra
    if (tex_color.a <= 0.05) {
        discard;
    }

    // Desenha a cor PURA da imagem (sem multiplicar por zero!)
    f = tex_color;
}