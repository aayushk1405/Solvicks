#!/bin/bash
# Make sure you have Emscripten activated (source ./emsdk_env.sh) before running this!

echo "Compiling C++ Engine to WebAssembly..."

emcc solver.cpp -o ../public/solver.js \
  -O3 \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s EXPORT_ES6=0 \
  -s MODULARIZE=1 \
  -s EXPORT_NAME="createModule" \
  --bind

echo "Build complete! solver.js and solver.wasm generated in /public folder."
