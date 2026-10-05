# 📸 Digitalização 3D & Fotogrametria — Visualizador e Gerador 3D

<p align="center">
  <b>Aplicação Web completa para visualização de modelos 3D locais (.glb), integração com Luma AI e geração por IA</b>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/React-19-blue?style=for-the-badge&logo=react" alt="React" />
  <img src="https://img.shields.io/badge/TypeScript-Strict-blue?style=for-the-badge&logo=typescript" alt="TypeScript" />
  <img src="https://img.shields.io/badge/Vite-Build%20Tool-yellow?style=for-the-badge&logo=vite" alt="Vite" />
  <img src="https://img.shields.io/badge/Three.js-react--three--fiber-black?style=for-the-badge&logo=three.js" alt="Three.js" />
  <img src="https://img.shields.io/badge/Luma-Gaussian%20Splatting-orange?style=for-the-badge" alt="Luma AI" />
</p>

---

## 📖 Visão Geral

O **Visualizador e Gerador 3D** é uma aplicação web construída em **React + TypeScript + Vite + Three.js** voltada para o ecossistema de digitalização 3D e fotogrametria com mesa giratória automatizada (ESP32).

O projeto oferece **3 modos de operação**:

1. 📦 **Importar Modelo 3D Local (.glb / .gltf)**:
   - Carregamento instantâneo de modelos exportados de softwares como **Luma Labs**, **3DF Zephyr**, **Meshroom** ou **Blender**.
   - Funciona 100% offline, direto no navegador via Three.js (sem depender de chave de API).
2. ✨ **Visualizador Luma AI (NeRF / Gaussian Splatting)**:
   - Permite colar o link da captura gerada no [lumalabs.ai](https://lumalabs.ai) a partir do conjunto de 24 fotos do ESP32 para navegação 3D interativa de alta fidelidade.
3. 📸 **Gerador por Fotos (API)**:
   - Captura de fotos pela câmera ou upload de imagens do dispositivo para envio a serviços de IA.

---

## ✨ Funcionalidades

- 🧊 **Visualizador 3D com Three.js / R3F** — iluminação de estúdio, controle orbital (zoom, rotação e pan) e liberação automática de memória.
- 📁 **Dropzone de Arquivos 3D** — arraste e solte arquivos `.glb` ou `.gltf` locais.
- 🌐 **Embed Integrado Luma AI** — suporte a links e IDs de captura da Luma com controle de cena completo.
- 📷 **Captura pela Câmera & Upload de Fotos** — tire fotos ou selecione do dispositivo.
- ⬇️ **Exportação e Download** — baixe os arquivos `.glb` para uso em qualquer software 3D.

---

## 🚀 Como Rodar o Projeto

```bash
# 1. Entre na pasta do frontend
cd gerador-3d

# 2. Instale as dependências
npm install

# 3. Inicie o servidor de desenvolvimento
npm run dev
```

Abra `http://localhost:5173` no navegador.
