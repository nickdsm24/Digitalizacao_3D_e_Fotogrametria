export type CapturedImage = {
  id: string;
  file: File;
  previewUrl: string;
  source: 'upload' | 'camera';
};