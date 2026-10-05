import { useState } from 'react';

type LumaViewerProps = {
  initialUrl?: string;
};

export function LumaViewer({ initialUrl = '' }: LumaViewerProps) {
  const [inputUrl, setInputUrl] = useState(initialUrl);
  const [activeUrl, setActiveUrl] = useState(initialUrl);

  function normalizeLumaEmbedUrl(raw: string): string {
    const trimmed = raw.trim();
    if (!trimmed) return '';

    // Se já for embed
    if (trimmed.includes('lumalabs.ai/embed/')) {
      return trimmed;
    }

    // Se for URL normal de capture: https://lumalabs.ai/capture/UUID
    const captureMatch = trimmed.match(/lumalabs\.ai\/capture\/([a-zA-Z0-9_-]+)/);
    if (captureMatch && captureMatch[1]) {
      return `https://lumalabs.ai/embed/${captureMatch[1]}?mode=sparkles&background=%2316181d`;
    }

    // Se for apenas o UUID
    if (/^[a-zA-Z0-9_-]{10,}$/.test(trimmed)) {
      return `https://lumalabs.ai/embed/${trimmed}?mode=sparkles&background=%2316181d`;
    }

    return trimmed;
  }

  function handleSubmit(e: React.FormEvent) {
    e.preventDefault();
    const normalized = normalizeLumaEmbedUrl(inputUrl);
    setActiveUrl(normalized);
  }

  const embedUrl = normalizeLumaEmbedUrl(activeUrl);

  return (
    <div className="luma-container">
      <form className="luma-form" onSubmit={handleSubmit}>
        <div className="luma-input-group">
          <input
            type="text"
            className="luma-input"
            placeholder="Cole o link da captura Luma AI (ex: https://lumalabs.ai/capture/...)"
            value={inputUrl}
            onChange={(e) => setInputUrl(e.target.value)}
          />
          <button type="submit" className="luma-submit-btn">
            Carregar Cena
          </button>
        </div>
        <p className="luma-help">
          💡 Suba o lote de 24 fotos do ESP32 no{' '}
          <a href="https://lumalabs.ai" target="_blank" rel="noreferrer">
            lumalabs.ai
          </a>
          , aguarde o processamento e cole o link gerado acima.
        </p>
      </form>

      <div className="viewer-stage luma-stage">
        {embedUrl ? (
          <iframe
            src={embedUrl}
            title="Luma AI 3D Capture Viewer"
            className="luma-iframe"
            allow="fullscreen; xr-spatial-tracking"
          />
        ) : (
          <div className="viewer-empty">
            <p>Nenhuma captura Luma carregada.</p>
            <span className="luma-tip">
              Cole a URL da sua captura do Luma AI para navegar no modelo em tempo real.
            </span>
          </div>
        )}
      </div>
    </div>
  );
}
