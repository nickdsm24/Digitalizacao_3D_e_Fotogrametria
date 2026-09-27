import { useCallback, useEffect, useRef, useState } from 'react';

type FacingMode = 'user' | 'environment';
type CameraError = 'permission-denied' | 'not-found' | 'unknown' | null;

export function useCamera() {
  const videoRef = useRef<HTMLVideoElement>(null);
  const streamRef = useRef<MediaStream | null>(null);

  const [isActive, setIsActive] = useState(false);
  const [facingMode, setFacingMode] = useState<FacingMode>('environment');
  const [error, setError] = useState<CameraError>(null);

  // Para os tracks da stream — essencial, senão a luz da câmera fica ligada.
  const stop = useCallback(() => {
    streamRef.current?.getTracks().forEach((track) => track.stop());
    streamRef.current = null;
    setIsActive(false);
  }, []);

  const start = useCallback(
    async (mode: FacingMode = facingMode) => {
      setError(null);
      stop(); // evita duas streams simultâneas (ex.: ao trocar de câmera)

      try {
        const stream = await navigator.mediaDevices.getUserMedia({
          video: { facingMode: mode },
          audio: false,
        });
        streamRef.current = stream;
        if (videoRef.current) {
          videoRef.current.srcObject = stream;
          await videoRef.current.play();
        }
        setFacingMode(mode);
        setIsActive(true);
      } catch (err) {
        if (err instanceof DOMException && err.name === 'NotAllowedError') {
          setError('permission-denied');
        } else if (err instanceof DOMException && err.name === 'NotFoundError') {
          setError('not-found');
        } else {
          setError('unknown');
        }
        setIsActive(false);
      }
    },
    [facingMode, stop],
  );

  const switchCamera = useCallback(() => {
    start(facingMode === 'environment' ? 'user' : 'environment');
  }, [facingMode, start]);

  // Desenha o frame atual do <video> num <canvas> e converte pra Blob.
  const capture = useCallback((): Promise<Blob | null> => {
    return new Promise((resolve) => {
      const video = videoRef.current;
      if (!video || video.videoWidth === 0) {
        resolve(null);
        return;
      }
      const canvas = document.createElement('canvas');
      canvas.width = video.videoWidth;
      canvas.height = video.videoHeight;
      const ctx = canvas.getContext('2d');
      if (!ctx) {
        resolve(null);
        return;
      }
      ctx.drawImage(video, 0, 0, canvas.width, canvas.height);
      canvas.toBlob((blob) => resolve(blob), 'image/jpeg', 0.92);
    });
  }, []);

  // Garante que a câmera para se o componente que usa o hook desmontar.
  useEffect(() => stop, [stop]);

  return { videoRef, isActive, error, facingMode, start, stop, switchCamera, capture };
}
