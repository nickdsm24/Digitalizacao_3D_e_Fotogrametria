import { useRef } from 'react';
import type { CapturedImage } from '../types';

const MAX_SIZE_BYTES = 10 * 1024 * 1024; // 10MB por foto

type ImageUploaderProps = {
  onImagesAdded: (images: CapturedImage[]) => void;
};

function createId() {
  return typeof crypto !== 'undefined' && 'randomUUID' in crypto
    ? crypto.randomUUID()
    : `${Date.now()}-${Math.random().toString(16).slice(2)}`;
}

export function ImageUploader({ onImagesAdded }: ImageUploaderProps) {
  const inputRef = useRef<HTMLInputElement>(null);

  function handleFiles(fileList: FileList | null) {
    if (!fileList || fileList.length === 0) return;

    const accepted: CapturedImage[] = [];
    const rejected: string[] = [];

    Array.from(fileList).forEach((file) => {
      if (!file.type.startsWith('image/')) {
        rejected.push(`${file.name} (tipo não suportado)`);
        return;
      }
      if (file.size > MAX_SIZE_BYTES) {
        rejected.push(`${file.name} (maior que 10MB)`);
        return;
      }
      accepted.push({
        id: createId(),
        file,
        previewUrl: URL.createObjectURL(file),
        source: 'upload',
      });
    });

    if (accepted.length > 0) {
      onImagesAdded(accepted);
    }

    if (rejected.length > 0) {
      // Feedback simples por enquanto; pode virar um toast/estado de erro depois.
      console.warn('Imagens ignoradas:', rejected.join(', '));
    }

    // Permite selecionar o mesmo arquivo de novo depois de removê-lo.
    if (inputRef.current) {
      inputRef.current.value = '';
    }
  }

  return (
    <>
      <button
        type="button"
        className="capture-tile"
        onClick={() => inputRef.current?.click()}
      >
        <UploadIcon />
        <span className="tile-title">Enviar fotos</span>
        <span className="tile-hint">Escolha imagens do seu dispositivo</span>
      </button>
      <input
        ref={inputRef}
        type="file"
        accept="image/*"
        multiple
        hidden
        onChange={(e) => handleFiles(e.target.files)}
      />
    </>
  );
}

function UploadIcon() {
  return (
    <svg width="22" height="22" viewBox="0 0 22 22" fill="none" aria-hidden="true">
      <path d="M11 14V3M11 3l-4.5 4.5M11 3l4.5 4.5" stroke="currentColor" strokeWidth="1.6" strokeLinecap="round" strokeLinejoin="round" />
      <path d="M3 14v3a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2v-3" stroke="currentColor" strokeWidth="1.6" strokeLinecap="round" strokeLinejoin="round" />
    </svg>
  );
}