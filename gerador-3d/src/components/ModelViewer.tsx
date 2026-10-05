import { Suspense, useEffect } from 'react';
import { Canvas } from '@react-three/fiber';
import { OrbitControls, Stage, useGLTF, Html, useProgress } from '@react-three/drei';

type ModelProps = {
  url: string;
};

function Model({ url }: ModelProps) {
  const { scene } = useGLTF(url);
  return <primitive object={scene} />;
}

function Loader() {
  const { progress } = useProgress();
  return (
    <Html center>
      <span className="viewer-loading">Carregando modelo… {Math.round(progress)}%</span>
    </Html>
  );
}

type ModelViewerProps = {
  modelUrl: string;
};

export function ModelViewer({ modelUrl }: ModelViewerProps) {
  // Libera a geometria/textura em cache ao trocar de modelo ou desmontar —
  // evita leak de memória se o usuário gerar vários modelos na mesma sessão.
  useEffect(() => {
    return () => {
      useGLTF.clear(modelUrl);
    };
  }, [modelUrl]);

  return (
    <Canvas camera={{ position: [0, 0, 3], fov: 50 }}>
      <Suspense fallback={<Loader />}>
        <Stage environment="studio" intensity={0.6}>
          <Model url={modelUrl} />
        </Stage>
        <OrbitControls makeDefault />
      </Suspense>
    </Canvas>
  );
}