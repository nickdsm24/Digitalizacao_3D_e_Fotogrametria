import { useState } from 'react';
import './App.css';
import { ImageUploader } from './components/ImageUploader';
import { CameraCapture } from './components/CameraCapture';
import { ModelViewer } from './components/ModelViewer';
import { useModelGeneration, type GenerationStatus } from './hooks/useModelGeneration';
import type { CapturedImage } from './types';

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
  const [images, setImages] = useState<CapturedImage[]>([]);
  const hasImages = images.length > 0;

  const { status, modelUrl, errorMessage, generate } = useModelGeneration();
  const isBusy = status === 'uploading' || status === 'queued' || status === 'processing';

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

  return (
    <div className="page">
      <header className="site-header">
        <div className="brand">
          <CubeMark />
          <span className="brand-name">Modelo3D</span>
        </div>
        <h1>Transforme fotos em modelos 3D</h1>
        <p className="lede">
          Envie ou tire uma foto do objeto e receba um modelo 3D navegável em poucos minutos.
        </p>
      </header>

      <main className="content">
        <section className="capture" aria-label="Adicionar fotos">
          <ImageUploader onImagesAdded={handleImagesAdded} />
          <CameraCapture onImagesAdded={handleImagesAdded} />
        </section>

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
            {isBusy ? 'Gerando…' : 'Gerar modelo 3D'}
          </button>
          {status !== 'idle' && (
            <p className={`status-line status-${status}`} role="status">
              {statusMessage(status, errorMessage)}
            </p>
          )}
        </section>

        <section className="viewer" aria-label="Modelo 3D gerado">
          <div className="viewer-stage">
            {modelUrl ? (
              <div className="viewer-canvas">
                <ModelViewer modelUrl={modelUrl} />
              </div>
            ) : (
              <p className="viewer-empty">O modelo 3D aparece aqui depois de gerado.</p>
            )}
          </div>
          {modelUrl && (
            <a href={modelUrl} download className="viewer-download">
              Baixar modelo (.glb)
            </a>
          )}
        </section>
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