import React, { useState, useEffect, useRef } from 'react';
import * as THREE from 'three';

// --- 3D Visual Component ---
function SpinningCube() {
  const mountRef = useRef(null);

  useEffect(() => {
    const currentMount = mountRef.current;
    if (!currentMount) return;

    // 1. Scene Setup
    const scene = new THREE.Scene();

    // 2. Camera Setup
    const camera = new THREE.PerspectiveCamera(
      75,
      currentMount.clientWidth / currentMount.clientHeight,
      0.1,
      1000
    );
    camera.position.set(4, 4, 4);
    camera.lookAt(0, 0, 0);

    // 3. Renderer Setup
    const renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
    renderer.setSize(currentMount.clientWidth, currentMount.clientHeight);
    renderer.setPixelRatio(window.devicePixelRatio);
    currentMount.appendChild(renderer.domElement);

    // 4. Lights Setup
    const ambientLight = new THREE.AmbientLight(0xffffff, 0.8);
    scene.add(ambientLight);

    const pointLight = new THREE.PointLight(0xffffff, 1.5);
    pointLight.position.set(10, 10, 10);
    scene.add(pointLight);

    // 5. Meshes Setup (Wireframe + Solid Core)
    const group = new THREE.Group();

    const wireframeGeometry = new THREE.BoxGeometry(2.5, 2.5, 2.5);
    const wireframeMaterial = new THREE.MeshStandardMaterial({
      color: 0x2d3748,
      wireframe: true,
    });
    const wireframeCube = new THREE.Mesh(wireframeGeometry, wireframeMaterial);
    group.add(wireframeCube);

    const solidGeometry = new THREE.BoxGeometry(2.4, 2.4, 2.4);
    const solidMaterial = new THREE.MeshStandardMaterial({ color: 0x4a5568 });
    const solidCube = new THREE.Mesh(solidGeometry, solidMaterial);
    group.add(solidCube);

    scene.add(group);

    // 6. Animation Loop
    let animationFrameId;
    const animate = () => {
      group.rotation.x += 0.016 * 0.2; 
      group.rotation.y += 0.016 * 0.3;

      renderer.render(scene, camera);
      animationFrameId = requestAnimationFrame(animate);
    };
    animate();

    // 7. Handle Resizing
    const handleResize = () => {
      if (!currentMount) return;
      camera.aspect = currentMount.clientWidth / currentMount.clientHeight;
      camera.updateProjectionMatrix();
      renderer.setSize(currentMount.clientWidth, currentMount.clientHeight);
    };
    window.addEventListener('resize', handleResize);

    // 8. Cleanup
    return () => {
      window.removeEventListener('resize', handleResize);
      cancelAnimationFrame(animationFrameId);
      if (currentMount.contains(renderer.domElement)) {
        currentMount.removeChild(renderer.domElement);
      }
      
      wireframeGeometry.dispose();
      wireframeMaterial.dispose();
      solidGeometry.dispose();
      solidMaterial.dispose();
      renderer.dispose();
    };
  }, []);

  return <div ref={mountRef} className="w-full h-full" />;
}

// --- Main Application Component ---
export default function App() {
  const [wasmModule, setWasmModule] = useState(null);
  const [method, setMethod] = useState('BFS');
  const [cubeState, setCubeState] = useState('UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB');
  const [solution, setSolution] = useState('');
  const [isWasmReady, setIsWasmReady] = useState(false);

  // Load Emscripten WASM Module dynamically
  useEffect(() => {
    const script = document.createElement('script');
    script.src = '/solver.js';
    script.async = true;
    script.onload = () => {
      if (window.createModule) {
        window.createModule().then((module) => {
          setWasmModule(module);
          setIsWasmReady(true);
        });
      }
    };
    document.body.appendChild(script);
    
    const fallbackTimeout = setTimeout(() => {
        if (!window.createModule) {
            setSolution("Warning: /solver.js not found. Make sure you ran the build.sh script.");
        }
    }, 3000);

    return () => {
      document.body.removeChild(script);
      clearTimeout(fallbackTimeout);
    };
  }, []);

  const handleSolve = () => {
    if (!isWasmReady) return;
    setSolution("Computing... (Running C++ Engine natively)");
    
    setTimeout(() => {
      const startTime = performance.now();
      try {
        const result = wasmModule.universalSolver(cubeState, method);
        const endTime = performance.now();
        setSolution(`Result: ${result}\nComputed in ${(endTime - startTime).toFixed(2)} ms`);
      } catch (err) {
        setSolution(`Error invoking WASM solver: ${err.message}`);
      }
    }, 50);
  };

  const handleScramble = () => {
    // A 4-move scramble that BFS can solve quickly
    setCubeState('UURUUUUUURRRRRRRRRDFFFFFFFFFDDDDDDDDLLLLLLLLLBBBBBBBBB'); 
    setSolution("Cube Scrambled! Ready to solve.");
  };

  return (
    <div className="min-h-screen bg-gray-900 text-white font-sans flex flex-col items-center justify-center p-6">
      
      <div className="w-full max-w-4xl bg-gray-800 rounded-xl shadow-2xl overflow-hidden border border-gray-700">
        
        {/* Header */}
        <div className="bg-gray-900 p-6 border-b border-gray-700 text-center">
          <h1 className="text-3xl font-bold bg-clip-text text-transparent bg-gradient-to-r from-blue-400 to-emerald-400">
            WebAssembly C++ Rubik's Engine
          </h1>
          <p className="text-gray-400 mt-2">High-performance algorithmic solver running in the browser.</p>
        </div>

        <div className="flex flex-col md:flex-row h-[500px]">
          
          {/* 3D Visualization Pane */}
          <div className="md:w-1/2 bg-gray-950 relative border-r border-gray-700">
            <SpinningCube />
            
            {!isWasmReady && (
              <div className="absolute inset-0 flex items-center justify-center bg-black bg-opacity-70 z-10">
                <span className="text-yellow-400 font-bold animate-pulse">Initializing C++ Engine...</span>
              </div>
            )}
          </div>

          {/* Controls Pane */}
          <div className="md:w-1/2 p-8 flex flex-col justify-between">
            
            <div className="space-y-6">
              <div>
                <label className="block text-sm font-medium text-gray-400 mb-2">Algorithm Strategy</label>
                <select 
                  className="w-full bg-gray-700 border border-gray-600 rounded-lg p-3 text-white focus:ring-2 focus:ring-blue-500 outline-none"
                  value={method}
                  onChange={(e) => setMethod(e.target.value)}
                >
                  <option value="BFS">Breadth-First Search (Short Scrambles)</option>
                  <option value="LAYER">Layer-by-Layer (Human Method - Stub)</option>
                  <option value="IDASTAR">IDA* (Advanced Heuristic Search)</option>
                </select>
              </div>

              <div className="flex gap-4">
                <button 
                  onClick={handleScramble}
                  disabled={!isWasmReady}
                  className="flex-1 bg-gray-700 hover:bg-gray-600 text-white font-semibold py-3 px-4 rounded-lg transition-colors disabled:opacity-50 disabled:cursor-not-allowed"
                >
                  Test Scramble
                </button>
                <button 
                  onClick={handleSolve}
                  disabled={!isWasmReady}
                  className="flex-1 bg-blue-600 hover:bg-blue-500 text-white font-bold py-3 px-4 rounded-lg shadow-lg shadow-blue-500/30 transition-all active:scale-95 disabled:opacity-50 disabled:cursor-not-allowed"
                >
                  Execute Solver
                </button>
              </div>
            </div>

            {/* Output Console */}
            <div className="mt-8">
              <label className="block text-sm font-medium text-gray-400 mb-2">Engine Output Console</label>
              <div className="bg-black rounded-lg p-4 h-32 font-mono text-sm overflow-y-auto border border-gray-700 whitespace-pre-wrap text-emerald-400">
                {solution || "> Ready for input...\n> Select algorithm and execute."}
              </div>
            </div>

          </div>
        </div>
      </div>
      
    </div>
  );
}
