import { useRef, useState, type DragEvent, type ChangeEvent } from 'react';

export type Model3DFormat = 'glb' | 'gltf' | 'obj';

type ModelFileUploaderProps = {
  onModelSelected: (modelUrl: string, fileName: string, format: Model3DFormat) => void;
};

export function ModelFileUploader({ onModelSelected }: ModelFileUploaderProps) {
  const [isDragging, setIsDragging] = useState(false);
  const [selectedFileName, setSelectedFileName] = useState<string | null>(null);
  const inputRef = useRef<HTMLInputElement>(null);

  function handleFile(file: File) {
    const lowerName = file.name.toLowerCase();
    let format: Model3DFormat | null = null;

    if (lowerName.endsWith('.glb')) format = 'glb';
    else if (lowerName.endsWith('.gltf')) format = 'gltf';
    else if (lowerName.endsWith('.obj')) format = 'obj';

    if (!format) {
      alert('Formato não suportado. Por favor, envie um arquivo .obj, .glb ou .gltf');
      return;
    }

    const objectUrl = URL.createObjectURL(file);
    setSelectedFileName(file.name);
    onModelSelected(objectUrl, file.name, format);
  }

  function handleDrop(e: DragEvent<HTMLDivElement>) {
    e.preventDefault();
    setIsDragging(false);

    if (e.dataTransfer.files && e.dataTransfer.files.length > 0) {
      handleFile(e.dataTransfer.files[0]);
    }
  }

  function handleDragOver(e: DragEvent<HTMLDivElement>) {
    e.preventDefault();
    setIsDragging(true);
  }

  function handleDragLeave() {
    setIsDragging(false);
  }

  function handleChange(e: ChangeEvent<HTMLInputElement>) {
    if (e.target.files && e.target.files.length > 0) {
      handleFile(e.target.files[0]);
    }
  }

  return (
    <div
      className={`model-dropzone ${isDragging ? 'dragging' : ''}`}
      onDrop={handleDrop}
      onDragOver={handleDragOver}
      onDragLeave={handleDragLeave}
      onClick={() => inputRef.current?.click()}
      role="button"
      tabIndex={0}
      onKeyDown={(e) => {
        if (e.key === 'Enter' || e.key === ' ') {
          e.preventDefault();
          inputRef.current?.click();
        }
      }}
    >
      <input
        ref={inputRef}
        type="file"
        accept=".glb,.gltf,.obj"
        style={{ display: 'none' }}
        onChange={handleChange}
      />

      <div className="dropzone-content">
        <div className="dropzone-icon">
          <svg width="40" height="40" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.5">
            <path d="M21 16V8a2 2 0 0 0-1-1.73l-7-4a2 2 0 0 0-2 0l-7 4A2 2 0 0 0 3 8v8a2 2 0 0 0 1 1.73l7 4a2 2 0 0 0 2 0l7-4A2 2 0 0 0 21 16z" />
            <polyline points="3.27 6.96 12 12.01 20.73 6.96" />
            <line x1="12" y1="22.08" x2="12" y2="12" />
          </svg>
        </div>
        <div className="dropzone-text">
          <strong>
            {selectedFileName ? `Arquivo: ${selectedFileName}` : 'Arraste ou clique para carregar seu modelo 3D (.obj / .glb / .gltf)'}
          </strong>
          <span className="dropzone-hint">
            Compatível com modelos exportados do 3DF Zephyr, Meshroom, Luma Labs ou Blender
          </span>
        </div>
      </div>
    </div>
  );
}
