import { useState } from 'react';
import './App.css';
import { ImageUploader } from './components/ImageUploader';
import { CameraCapture } from './components/CameraCapture';
import { ModelViewer } from './components/ModelViewer';
import { ModelFileUploader, type Model3DFormat } from './components/ModelFileUploader';
import { LumaViewer } from './components/LumaViewer';
import { useModelGeneration, type GenerationStatus } from './hooks/useModelGeneration';
import type { CapturedImage } from './types';

type ActiveTab = 'local-model' | 'luma-viewer' | 'photo-generator';

function statusMessage(status: GenerationStatus, errorMessage: string | null): string {
  switch (status) {
    case 'uploading':
      return 'Enviando fotos…';
    case 'queued':
      return 'Na fila de processamento…';
    case 'processing':
      return 'Gerando modelo 3D…';
    case 'done':
      return 'Modelo gerado com sucesso!';
    case 'error':
      return errorMessage ?? 'Ocorreu um erro ao gerar o modelo. Tente novamente.';
    default:
      return '';
  }
}

function App() {
  const [activeTab, setActiveTab] = useState<ActiveTab>('local-model');

  // Estado para fotos e geração por IA
  const [images, setImages] = useState<CapturedImage[]>([]);
  const hasImages = images.length > 0;

  const { status, modelUrl: apiModelUrl, errorMessage, generate } = useModelGeneration();
  const isBusy = status === 'uploading' || status === 'queued' || status === 'processing';

  // Estado para modelo local .obj / .glb importado
  const [localModelUrl, setLocalModelUrl] = useState<string | null>(null);
  const [localFileName, setLocalFileName] = useState<string | null>(null);
  const [localModelFormat, setLocalModelFormat] = useState<Model3DFormat>('glb');

  function handleImagesAdded(newImages: CapturedImage[]) {
    setImages((prev) => [...prev, ...newImages]);
  }

  function handleRemoveImage(id: string) {
    setImages((prev) => {
      const target = prev.find((img) => img.id === id);
      if (target) URL.revokeObjectURL(target.previewUrl);
      return prev.filter((img) => img.id !== id);
    });
  }

  function handleGenerate() {
    generate(images.map((img) => img.file));
  }

  function handleLocalModelSelected(url: string, fileName: string, format: Model3DFormat) {
    if (localModelUrl && localModelUrl.startsWith('blob:')) {
      URL.revokeObjectURL(localModelUrl);
    }
    setLocalModelUrl(url);
    setLocalFileName(fileName);
    setLocalModelFormat(format);
  }

  return (
    <div className="page">
      <header className="site-header">
        <div className="brand">
          <CubeMark />
          <span className="brand-name">Digitalização 3D & Fotogrametria</span>
        </div>
        <h1>Visualizador e Gerador 3D</h1>
        <p className="lede">
          Visualize modelos reconstruídos a partir das fotos da mesa giratória ou gere novos objetos 3D.
        </p>
      </header>

      {/* Navegação por Abas */}
      <nav className="tab-nav" aria-label="Modos de visualização">
        <button
          type="button"
          className={`tab-btn ${activeTab === 'local-model' ? 'active' : ''}`}
          onClick={() => setActiveTab('local-model')}
        >
          📦 Importar Modelo (.obj / .glb)
        </button>
        <button
          type="button"
          className={`tab-btn ${activeTab === 'luma-viewer' ? 'active' : ''}`}
          onClick={() => setActiveTab('luma-viewer')}
        >
          ✨ Luma AI (NeRF / Splatting)
        </button>
        <button
          type="button"
          className={`tab-btn ${activeTab === 'photo-generator' ? 'active' : ''}`}
          onClick={() => setActiveTab('photo-generator')}
        >
          📸 Gerador por Fotos (API)
        </button>
      </nav>

      <main className="content">
        {/* ABA 1: UPLOAD LOCAL DE .OBJ / .GLB */}
        {activeTab === 'local-model' && (
          <section className="tab-section" aria-label="Importar modelo 3D local">
            <ModelFileUploader onModelSelected={handleLocalModelSelected} />

            <div className="viewer" aria-label="Modelo 3D local">
              <div className="viewer-stage">
                {localModelUrl ? (
                  <div className="viewer-canvas">
                    <ModelViewer modelUrl={localModelUrl} format={localModelFormat} />
                  </div>
                ) : (
                  <div className="viewer-empty">
                    <p>Nenhum arquivo 3D carregado ainda.</p>
                    <span className="luma-tip">
                      Arraste um arquivo <strong>.obj</strong>, <strong>.glb</strong> ou <strong>.gltf</strong> exportado do 3DF Zephyr, Meshroom ou Luma.
                    </span>
                  </div>
                )}
              </div>
              {localModelUrl && (
                <div className="model-actions">
                  <a href={localModelUrl} download={localFileName || `modelo.${localModelFormat}`} className="viewer-download">
                    ⬇️ Baixar {localFileName || `modelo.${localModelFormat}`}
                  </a>
                  <button
                    type="button"
                    className="clear-model-btn"
                    onClick={() => {
                      if (localModelUrl.startsWith('blob:')) URL.revokeObjectURL(localModelUrl);
                      setLocalModelUrl(null);
                      setLocalFileName(null);
                    }}
                  >
                    Remover Modelo
                  </button>
                </div>
              )}
            </div>
          </section>
        )}

        {/* ABA 2: VISUALIZADOR LUMA AI */}
        {activeTab === 'luma-viewer' && (
          <section className="tab-section" aria-label="Visualizador Luma AI">
            <LumaViewer />
          </section>
        )}

        {/* ABA 3: GERAÇÃO POR FOTOS (API) */}
        {activeTab === 'photo-generator' && (
          <section className="tab-section" aria-label="Gerar modelo por fotos">
            <div className="capture">
              <ImageUploader onImagesAdded={handleImagesAdded} />
              <CameraCapture onImagesAdded={handleImagesAdded} />
            </div>

            <section className="preview" aria-label="Fotos selecionadas">
              {hasImages ? (
                <div className="preview-grid">
                  {images.map((img) => (
                    <div className="preview-item" key={img.id}>
                      <img src={img.previewUrl} alt="" className="preview-thumb" />
                      <button
                        type="button"
                        className="preview-remove"
                        aria-label="Remover foto"
                        onClick={() => handleRemoveImage(img.id)}
                      >
                        ×
                      </button>
                    </div>
                  ))}
                </div>
              ) : (
                <p className="empty-hint">Suas fotos aparecem aqui depois de selecionadas.</p>
              )}
            </section>

            <section className="generate">
              <button
                type="button"
                className="generate-button"
                disabled={!hasImages || isBusy}
                onClick={handleGenerate}
              >
                {isBusy ? 'Gerando…' : 'Gerar modelo 3D via API'}
              </button>
              {status !== 'idle' && (
                <p className={`status-line status-${status}`} role="status">
                  {statusMessage(status, errorMessage)}
                </p>
              )}
            </section>

            <section className="viewer" aria-label="Modelo 3D gerado via API">
              <div className="viewer-stage">
                {apiModelUrl ? (
                  <div className="viewer-canvas">
                    <ModelViewer modelUrl={apiModelUrl} format="glb" />
                  </div>
                ) : (
                  <p className="viewer-empty">O modelo 3D aparece aqui depois de gerado pela API.</p>
                )}
              </div>
              {apiModelUrl && (
                <a href={apiModelUrl} download className="viewer-download">
                  ⬇️ Baixar modelo (.glb)
                </a>
              )}
            </section>
          </section>
        )}
      </main>
    </div>
  );
}

function CubeMark() {
  return (
    <svg width="28" height="28" viewBox="0 0 28 28" fill="none" aria-hidden="true">
      <path d="M14 2 L25 8 L25 20 L14 26 L3 20 L3 8 Z" stroke="currentColor" strokeWidth="1.5" strokeLinejoin="round" />
      <path d="M14 14 L14 2 M14 14 L25 20 M14 14 L3 20" stroke="currentColor" strokeWidth="1.5" strokeLinejoin="round" />
    </svg>
  );
}

export default App;