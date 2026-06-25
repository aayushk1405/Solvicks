# Solvix — WebAssembly C++ Rubik's Cube Engine

A browser-based Rubik's Cube solver where the actual solving logic runs as **C++ compiled to WebAssembly**, not JavaScript. The React frontend renders a live 3D cube and hands off solving to a native WASM module via Emscripten bindings.

![Tech](https://img.shields.io/badge/engine-C%2B%2B%20%2B%20WASM-blue) ![Frontend](https://img.shields.io/badge/frontend-React%20%2B%20Vite-61DAFB) ![Render](https://img.shields.io/badge/3D-Three.js-black)

---

## What it does

- Renders an interactive, accurately colored 3D Rubik's Cube (Three.js) driven directly by the solver's internal state representation.
- Scrambles the cube by applying a real sequence of legal moves (guaranteed solvable — not a hand-typed fake state).
- Solves the cube using one of two algorithms, executed natively in WebAssembly:
  - **Breadth-First Search (BFS)** — optimal for short scrambles, exhaustive otherwise.
  - **IDA\* (Iterative Deepening A\*)** — heuristic-guided search that scales further than BFS before timing out.
- Reports the solve time and move sequence in a console-style output panel.

> **Layer-by-Layer (human method)** is listed in the UI as a disabled option — it is not implemented. It would require ~50+ hand-coded macros (cross, F2L, OLL, PLL) and is left as a stub on purpose.

---

## Tech Stack

| Layer | Technology |
|---|---|
| Solving engine | C++17, compiled with Emscripten (`emcc`) to WebAssembly |
| JS ⇄ WASM bridge | `embind` (Emscripten bindings) |
| Frontend framework | React 18 + Vite |
| 3D rendering | Three.js (raw, no React Three Fiber wrapper currently in use) |
| Styling | Tailwind CSS |

---

## Project Structure

```
Solvix/
├── src/
│   ├── App.jsx          # Main React app: UI, state, 3D cube, WASM glue
│   ├── main.jsx          # React entry point
│   ├── index.css         # Tailwind base styles
│   ├── solver.cpp        # The actual solver — BFS, IDA*, cube move logic
│   └── build.sh          # Compiles solver.cpp -> public/solver.{js,wasm}
├── public/
│   ├── solver.js          # Emscripten-generated JS glue (generated, not hand-written)
│   └── solver.wasm        # Compiled WASM binary (generated)
├── index.html
├── package.json
├── vite.config.js
├── tailwind.config.js
└── postcss.config.js
```

---

## Cube State Representation

The cube is represented as a 54-character string of facelet colors, ordered face-by-face, 9 facelets per face, row-major (top-left to bottom-right as viewed from outside that face):

```
Indices  0–8   : U (Up)
Indices  9–17  : R (Right)
Indices 18–26  : F (Front)
Indices 27–35  : D (Down)
Indices 36–44  : L (Left)
Indices 45–53  : B (Back)
```

Solved state:
```
UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB
```

Each character is one of `U R F D L B`, representing sticker color (white/red/green/yellow/orange/blue respectively in the 3D view) — **not** the face it currently sits on. A valid cube state always has exactly 9 of each letter; anything else is not reachable by any sequence of legal moves.

Moves supported: `U R F D L B` and their primes (`U' R' F' D' L' B'`), implemented as 3× the clockwise rotation.

---

## Getting Started

### Prerequisites

- **Node.js** (v18+ recommended) and npm
- **[Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)** — required only if you need to rebuild the WASM binary (e.g. after editing `solver.cpp`). Not required just to run the app, since `public/solver.js` / `public/solver.wasm` are already committed as build artifacts.

### Install & Run

```bash
npm install
npm run dev
```

Open the printed local URL (typically `http://localhost:5173`).

### Building for production

```bash
npm run build
npm run preview   # optional: preview the production build locally
```

### Rebuilding the WASM engine

Only needed if you change `src/solver.cpp`.

```bash
# Activate Emscripten in your shell first
source /path/to/emsdk/emsdk_env.sh

cd src
./build.sh
```

This regenerates `public/solver.js` and `public/solver.wasm`. Commit both after rebuilding.

---

## How the Algorithms Work

### BFS (`solveBFS`)
Standard breadth-first search over cube states, exploring all 12 possible moves per node, using a hash map for visited-state parent tracking to reconstruct the solution path. Capped at 15,000 explored nodes (`TIMEOUT` beyond that) — appropriate for scrambles a few moves deep, but it does not scale to a fully shuffled cube (the state space is far too large for breadth-first exhaustive search).

### IDA\* (`solveIDAStar`)
Iterative-deepening A\*, using an admissible heuristic based on the count of misplaced facelets divided by the maximum number of facelets a single turn can affect (20), giving a lower bound on remaining moves. Search depth is capped at 12 and total node exploration capped at 500,000 to keep the browser responsive. This scales further than BFS but is still a simplified heuristic — nowhere near as tight as something like Kociemba's two-phase algorithm, so deep scrambles can still legitimately time out.

### Why both exist
This is a demonstration / learning project about running real search algorithms compiled to WASM in the browser, not a production-grade solver. For genuinely fast, optimal solving of any scramble, a proper Kociemba implementation (group-theoretic phase reduction + precomputed pruning tables) would be the next step — that's out of scope here.

---

## Known Limitations

- **Layer-by-Layer is not implemented.** The dropdown option exists but is disabled; selecting it isn't possible from the UI.
- **IDA\*** is heuristically weak (simple misplaced-facelet count) compared to real-world solvers, so it can time out on cubes that are scrambled past roughly God's-number-adjacent depths within the node/depth caps.
- **BFS** is only practical for scrambles of a few moves — this is expected and documented in-app via the dropdown label.
- The 3D viewer is a direct facelet-sticker rendering (54 colored planes on a black core), not a fully animated turn-by-turn move player — moves apply instantly to the whole cube state rather than animating individual layer rotations.

---

## Contributing / Extending

Ideas for follow-up work:
- Implement a real Layer-by-Layer (CFOP-lite) solver as macros operating on the facelet string.
- Replace the misplaced-facelet heuristic with a proper corner/edge pattern-database heuristic for IDA\*.
- Animate individual face turns in the 3D view instead of snapping to the new state instantly.
- Add a move-input UI so users can scramble manually instead of only via the randomized "Test Scramble" button.

---

## License

Add your preferred license here (e.g. MIT) — none specified yet.
