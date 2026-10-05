import { Suspense, useEffect } from 'react';
import { Canvas, useLoader } from '@react-three/fiber';
import { OrbitControls, Stage, useGLTF, Html, useProgress } from '@react-three/drei';
import { OBJLoader } from 'three/examples/jsm/loaders/OBJLoader.js';
import * as THREE from 'three';

type ModelProps = {
  url: string;
  format?: 'glb' | 'gltf' | 'obj';
};

function GltfModel({ url }: { url: string }) {
  const { scene } = useGLTF(url);
  return <primitive object={scene} />;
}

function ObjModel({ url }: { url: string }) {
  const obj = useLoader(OBJLoader, url);

  useEffect(() => {
    obj.traverse((child) => {
      if ((child as THREE.Mesh).isMesh) {
        const mesh = child as THREE.Mesh;
        mesh.castShadow = true;
        mesh.receiveShadow = true;

        // Se o OBJ não tiver material ou cor, aplica um material padrão para renderizar com luz
        if (!mesh.material || (Array.isArray(mesh.material) && mesh.material.length === 0)) {
          mesh.material = new THREE.MeshStandardMaterial({
            color: '#7bc876',
            roughness: 0.4,
            metalness: 0.1,
            side: THREE.DoubleSide,
          });
        }
      }
    });
  }, [obj]);

  return <primitive object={obj} />;
}

function ModelContent({ url, format }: ModelProps) {
  const isObj = format === 'obj' || url.toLowerCase().includes('.obj');

  if (isObj) {
    return <ObjModel url={url} />;
  }

  return <GltfModel url={url} />;
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
  format?: 'glb' | 'gltf' | 'obj';
};

export function ModelViewer({ modelUrl, format }: ModelViewerProps) {
  useEffect(() => {
    return () => {
      if (format !== 'obj') {
        useGLTF.clear(modelUrl);
      }
      THREE.Cache.remove(modelUrl);
    };
  }, [modelUrl, format]);

  return (
    <Canvas camera={{ position: [0, 0, 3], fov: 50 }}>
      <Suspense fallback={<Loader />}>
        <Stage environment="studio" intensity={0.6}>
          <ModelContent url={modelUrl} format={format} />
        </Stage>
        <OrbitControls makeDefault />
      </Suspense>
    </Canvas>
  );
}