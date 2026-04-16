# Light Dock

Um dock minimalista para Wayland (wlroots) baseado em C/C++.

⚠️ **Status: PROJETO EM DESENVOLVIMENTO**

Este é um projeto **incompleto e experimental**. Funcionalidades podem estar quebradas, incompletas ou mudar sem aviso.

## 📋 Sobre

Light Dock é um compositor shell para Wayland que implementa um dock/painel usando layer-shell protocol e OpenGL ES. Utiliza protocolos Wayland como wlr-layer-shell para criar uma interface de desktop leve e eficiente.

## ⚠️ Limitações

- **Apenas compatível com wlroots** (Sway, Hyprland, Wayfire, etc.)
- **Não compatível com GNOME Wayland, KDE Plasma Wayland ou outros compositors que não usem wlroots**
- Funcionalidades em desenvolvimento - pode não compilar ou funcionar corretamente
- API e estrutura podem mudar drasticamente

## 🔧 Dependências

### Ubuntu/Debian

```bash
sudo apt-get install cmake libwayland-dev libegl-dev libgles2-mesa-dev libwayland-egl-backend-dev pkg-config build-essential
