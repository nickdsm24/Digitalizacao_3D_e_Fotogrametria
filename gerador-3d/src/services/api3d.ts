// Integração com a Tripo3D API v3.
// Docs: https://developers.tripo3d.com/en/docs/introduction
//
// Fluxo (Etapa 3.1 do plano):
// 1. POST /v3/files            -> upload da imagem, retorna file_token
// 2. POST /v3/generation/image-to-model -> cria a tarefa, retorna task_id
// 3. GET  /v3/tasks/{task_id}  -> polling até status "success" | "failed"

// Chamar https://openapi.tripo3d.com direto do navegador é bloqueado por
// CORS — a Tripo3D não envia Access-Control-Allow-Origin (a doc deles é
// explícita: a API é pra uso server-side). Por padrão, batemos num caminho
// relativo que o proxy do vite.config.ts repassa pro domínio real durante
// o desenvolvimento (`npm run dev` / `npm run preview`).
// Em produção (build estático), esse caminho relativo precisa apontar pra
// um backend/serverless function seu que faça esse mesmo proxy — veja o
// comentário na Etapa 3.3 do plano.
const API_BASE = import.meta.env.VITE_3D_API_BASE_URL || '/tripo-api/v3';
const API_KEY = import.meta.env.VITE_3D_API_KEY as string | undefined;

export type TripoTaskStatus = 'queued' | 'running' | 'success' | 'failed' | 'cancelled';

export type TaskResult = {
  taskId: string;
  status: TripoTaskStatus;
  progress: number;
  modelUrl?: string;
};

// Formato de resposta padrão da Tripo3D: { code: 0, data: {...} } no sucesso,
// { code: <>0, message: "..." } no erro.
type ApiEnvelope<T> = { code: number; data?: T; message?: string };

function authHeaders(extra?: Record<string, string>): HeadersInit {
  if (!API_KEY) {
    throw new Error(
      'VITE_3D_API_KEY não configurada. Adicione sua chave da Tripo3D no arquivo .env (veja .env.example).',
    );
  }
  return { Authorization: `Bearer ${API_KEY}`, ...extra };
}

async function parseEnvelope<T>(res: Response): Promise<T> {
  const json = (await res.json().catch(() => null)) as ApiEnvelope<T> | null;
  if (!json || typeof json.code !== 'number') {
    throw new Error('Resposta inesperada da API Tripo3D.');
  }
  if (json.code !== 0 || !json.data) {
    throw new Error(json.message ?? `Erro da API Tripo3D (código ${json.code}).`);
  }
  return json.data;
}

// Upload da imagem: obrigatório antes de gerar, já que enviamos como
// file_token em vez de expor a foto via URL pública.
async function uploadImage(file: File): Promise<string> {
  const formData = new FormData();
  formData.append('file', file);

  const res = await fetch(`${API_BASE}/files`, {
    method: 'POST',
    headers: authHeaders(),
    body: formData,
  });
  const data = await parseEnvelope<{ file_token: string }>(res);
  return data.file_token;
}

// Etapa 3.2: uploadAndGenerate.
// A Tripo3D gera o modelo a partir de UMA imagem por tarefa (endpoint
// image-to-model). Por enquanto usamos só a primeira foto selecionada.
// Se no futuro quiser combinar várias fotos em ângulos diferentes
// (fotogrametria), troque para POST /v3/generation/multiview-to-model,
// que aceita até 4 imagens (frente/costas/esquerda/direita).
export async function uploadAndGenerate(files: File[]): Promise<{ taskId: string }> {
  if (files.length === 0) {
    throw new Error('Selecione ao menos uma foto antes de gerar o modelo.');
  }

  const [primary] = files;
  const fileToken = await uploadImage(primary);

  const res = await fetch(`${API_BASE}/generation/image-to-model`, {
    method: 'POST',
    headers: authHeaders({ 'Content-Type': 'application/json' }),
    body: JSON.stringify({
      input: fileToken,
      // A API espera a string de versão exata, não um alias como "tripo-v3.1".
      // Valores aceitos hoje: P1-20260311, P2-20260801, v2.5-20250123,
      // v3.0-20250812, v3.1-20260211. Se a Tripo3D lançar uma nova versão,
      // o erro 400 retorna a lista atualizada em `allowed values`.
      model: 'v3.1-20260211',
      texture: true,
      pbr: true,
    }),
  });
  const data = await parseEnvelope<{ task_id: string }>(res);
  return { taskId: data.task_id };
}

// Etapa 3.2: checkStatus, usado pelo polling em useModelGeneration.
export async function checkStatus(taskId: string): Promise<TaskResult> {
  const res = await fetch(`${API_BASE}/tasks/${taskId}`, {
    headers: authHeaders(),
  });
  const data = await parseEnvelope<{
    task_id: string;
    status: TripoTaskStatus;
    progress: number;
    output?: { model_url?: string; rendered_image_url?: string };
  }>(res);

  return {
    taskId: data.task_id,
    status: data.status,
    progress: data.progress,
    modelUrl: data.output?.model_url,
  };
}