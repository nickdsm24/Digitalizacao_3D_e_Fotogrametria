import { useState } from 'react';
import { useCamera } from '../hooks/useCamera';
import type { CapturedImage } from '../types';

type CameraCaptureProps = {
  onImagesAdded: (images: CapturedImage[]) => void;
};

function createId() {
  return typeof crypto !== 'undefined' && 'randomUUID' in crypto
    ? crypto.randomUUID()
    : `${Date.now()}-${Math.random().toString(16).slice(2)}`;
}

export function CameraCapture({ onImagesAdded }: CameraCaptureProps) {
  const [isOpen, setIsOpen] = useState(false);
  const [preview, setPreview] = useState<{ blob: Blob; url: string } | null>(null);
  const { videoRef, isActive, error, start, stop, switchCamera, capture } = useCamera();

  function handleOpen() {
    setPreview(null);
    setIsOpen(true);
    start();
  }

  function closeAndReset() {
    stop();
    setIsOpen(false);
    if (preview) URL.revokeObjectURL(preview.url);
    setPreview(null);
  }

  async function handleCapture() {
    const blob = await capture();
    if (!blob) return;
    setPreview({ blob, url: URL.createObjectURL(blob) });
  }

  function handleRetake() {
    if (preview) URL.revokeObjectURL(preview.url);
    setPreview(null);
  }

  function handleConfirm() {
    if (!preview) return;
    const file = new File([preview.blob], `foto-${Date.now()}.jpg`, { type: 'image/jpeg' });
    onImagesAdded([
      { id: createId(), file, previewUrl: preview.url, source: 'camera' },
    ]);
    stop();
    setIsOpen(false);
    setPreview(null);
  }

  return (
    <>
      <button type="button" className="capture-tile" onClick={handleOpen}>
        <CameraIcon />
        <span className="tile-title">Tirar foto</span>
        <span className="tile-hint">Use a câmera agora</span>
      </button>

      {isOpen && (
        <div className="camera-modal" role="dialog" aria-modal="true" aria-label="Câmera">
          <div className="camera-modal-content">
            <button
              type="button"
              className="camera-btn-close"
              onClick={closeAndReset}
              aria-label="Fechar câmera"
            >
              ×
            </button>

            {error === 'permission-denied' && (
              <p className="camera-error">
                Permissão de câmera negada. Habilite o acesso nas configurações do navegador e
                tente de novo.
              </p>
            )}
            {error === 'not-found' && (
              <p className="camera-error">Nenhuma câmera encontrada neste dispositivo.</p>
            )}
            {error === 'unknown' && (
              <p className="camera-error">Não foi possível acessar a câmera.</p>
            )}

            {!error && !preview && (
              // eslint-disable-next-line jsx-a11y/media-has-caption
              <video ref={videoRef} className="camera-video" playsInline muted />
            )}

            {preview && (
              <img src={preview.url} alt="Prévia da foto capturada" className="camera-video" />
            )}

            <div className="camera-actions">
              {!preview ? (
                <>
                  <button
                    type="button"
                    className="camera-btn-secondary"
                    onClick={switchCamera}
                    disabled={!isActive}
                  >
                    Trocar câmera
                  </button>
                  <button
                    type="button"
                    className="camera-btn-primary"
                    onClick={handleCapture}
                    disabled={!isActive}
                  >
                    Capturar
                  </button>
                </>
              ) : (
                <>
                  <button type="button" className="camera-btn-secondary" onClick={handleRetake}>
                    Tirar de novo
                  </button>
                  <button type="button" className="camera-btn-primary" onClick={handleConfirm}>
                    Usar foto
                  </button>
                </>
              )}
            </div>
          </div>
        </div>
      )}
    </>
  );
}

function CameraIcon() {
  return (
    <svg width="22" height="22" viewBox="0 0 22 22" fill="none" aria-hidden="true">
      <path d="M3 7.5A1.5 1.5 0 0 1 4.5 6h2l1-1.6A1 1 0 0 1 8.35 4h5.3a1 1 0 0 1 .85.4L15.5 6h2A1.5 1.5 0 0 1 19 7.5v9A1.5 1.5 0 0 1 17.5 18h-13A1.5 1.5 0 0 1 3 16.5v-9Z" stroke="currentColor" strokeWidth="1.6" strokeLinejoin="round" />
      <circle cx="11" cy="12" r="3.2" stroke="currentColor" strokeWidth="1.6" />
    </svg>
  );
}
