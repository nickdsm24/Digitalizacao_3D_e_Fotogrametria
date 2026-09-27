import { useCallback, useEffect, useRef, useState } from 'react';
import { checkStatus, uploadAndGenerate, type TripoTaskStatus } from '../services/api3d';

// Mesmos nomes de fase já usados na Etapa 2.5 na UI, pra não precisar
// mudar App.tsx além de trocar a simulação pela chamada real.
export type GenerationStatus = 'idle' | 'uploading' | 'queued' | 'processing' | 'done' | 'error';

const POLL_INTERVAL_MS = 3000;

function mapTripoStatus(status: TripoTaskStatus): GenerationStatus {
  switch (status) {
    case 'queued':
      return 'queued';
    case 'running':
      return 'processing';
    case 'success':
      return 'done';
    case 'failed':
    case 'cancelled':
      return 'error';
    default:
      return 'processing';
  }
}

export function useModelGeneration() {
  const [status, setStatus] = useState<GenerationStatus>('idle');
  const [modelUrl, setModelUrl] = useState<string | null>(null);
  const [errorMessage, setErrorMessage] = useState<string | null>(null);

  // `cancelled` evita setState depois que o componente desmontou ou que um
  // novo generate() foi disparado por cima de um polling anterior.
  const cancelledRef = useRef(false);
  const pollTimeoutRef = useRef<number | null>(null);

  const clearPoll = useCallback(() => {
    if (pollTimeoutRef.current !== null) {
      window.clearTimeout(pollTimeoutRef.current);
      pollTimeoutRef.current = null;
    }
  }, []);

  const poll = useCallback(
    (taskId: string) => {
      async function tick() {
        if (cancelledRef.current) return;
        try {
          const result = await checkStatus(taskId);
          if (cancelledRef.current) return;

          const mapped = mapTripoStatus(result.status);
          setStatus(mapped);

          if (mapped === 'done') {
            setModelUrl(result.modelUrl ?? null);
            return;
          }
          if (mapped === 'error') {
            setErrorMessage('A geração do modelo falhou. Tente novamente.');
            return;
          }
          pollTimeoutRef.current = window.setTimeout(tick, POLL_INTERVAL_MS);
        } catch (err) {
          if (cancelledRef.current) return;
          setStatus('error');
          setErrorMessage(err instanceof Error ? err.message : 'Erro ao consultar status.');
        }
      }
      tick();
    },
    [],
  );

  const generate = useCallback(
    async (files: File[]) => {
      cancelledRef.current = false;
      clearPoll();
      setErrorMessage(null);
      setModelUrl(null);
      setStatus('uploading');

      try {
        const { taskId } = await uploadAndGenerate(files);
        if (cancelledRef.current) return;
        setStatus('queued');
        poll(taskId);
      } catch (err) {
        if (cancelledRef.current) return;
        setStatus('error');
        setErrorMessage(err instanceof Error ? err.message : 'Erro ao enviar as fotos.');
      }
    },
    [clearPoll, poll],
  );

  const reset = useCallback(() => {
    cancelledRef.current = true;
    clearPoll();
    setStatus('idle');
    setModelUrl(null);
    setErrorMessage(null);
  }, [clearPoll]);

  // Cancela o polling se o componente desmontar no meio da geração.
  useEffect(() => {
    return () => {
      cancelledRef.current = true;
      clearPoll();
    };
  }, [clearPoll]);

  return { status, modelUrl, errorMessage, generate, reset };
}