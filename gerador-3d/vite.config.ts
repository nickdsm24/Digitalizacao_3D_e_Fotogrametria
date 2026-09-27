import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

// https://vite.dev/config/
export default defineConfig({
  plugins: [react()],
  server: {
    proxy: {
      // A Tripo3D não envia cabeçalhos CORS pra chamadas do navegador
      // (a própria doc deles diz pra usar a API só a partir do servidor).
      // Em dev, o navegador chama esse caminho relativo no próprio
      // localhost:5173, e é o servidor Node do Vite — não o navegador —
      // quem faz a chamada real pra Tripo3D. Sem CORS envolvido.
      //
      // ⚠️ Domínio: a Tripo3D tem duas plataformas separadas —
      // openapi.tripo3d.ai (internacional) e openapi.tripo3d.com (China).
      // Uma chave criada numa NÃO funciona na outra (dá 401). Se sua conta
      // foi criada em platform.tripo3d.ai, use o target abaixo; se foi em
      // platform.tripo3d.com, troque para "https://openapi.tripo3d.com".
      '/tripo-api': {
        target: 'https://openapi.tripo3d.ai',
        changeOrigin: true,
        rewrite: (path) => path.replace(/^\/tripo-api/, ''),
      },
    },
  },
  preview: {
    proxy: {
      '/tripo-api': {
        target: 'https://openapi.tripo3d.ai',
        changeOrigin: true,
        rewrite: (path) => path.replace(/^\/tripo-api/, ''),
      },
    },
  },
});