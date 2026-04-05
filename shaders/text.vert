#version 300 es
precision mediump float;

// O C++ envia-nos um "pacote" de 4 números de uma vez:
// vertex.x = Posição X na tela
// vertex.y = Posição Y na tela
// vertex.z = Coordenada U (X na imagem do "Carimbo")
// vertex.w = Coordenada V (Y na imagem do "Carimbo")
layout (location = 0) in vec4 vertex;

// A nossa matriz Ortográfica (o tamanho da sua tela, ex: 1920x1080)
uniform mat4 pr;

// A variável que vai viajar até ao text.frag
out vec2 f_uv;

void main() {
    // 1. Calcula a posição física da letra no monitor
    gl_Position = pr * vec4(vertex.xy, 0.0, 1.0);

    // 2. Separa a parte da textura (U, V) e envia para o Fragment Shader pintar
    f_uv = vertex.zw;
}