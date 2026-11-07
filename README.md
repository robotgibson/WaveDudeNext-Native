# WaveGuide-Native 🌊 (C++ Photonics Studio)

A high-performance, hardware-accelerated **native C++20 port** of the WaveGuide wave physics engine framework. This application implements a multi-threaded Finite-Difference Time-Domain (FDTD) numerical solver to simulate 2D electromagnetic fields and explore high-speed all-optical logic architectures right on Linux.

## 🚀 Key Features

* **Parallelized FDTD Math Core:** Leverages OpenMP loop vectorization and explicit `__restrict` pointers to distribute 2D Laplacian matrix calculation stencils across all available CPU thread cores concurrently.
* **Dynamic Data-Driven Scenes:** Zero hardcoded circuit paths or conditional scene overrides inside the core physics solver. Emitters, phases, relative offsets, and amplitude ceilings are hot-reloaded dynamically from `config/scene_profiles.json` upon runtime preset selection or first application launch.
* **Decoupled Analytics Footer Suite:** The bottom real-time telemetry diagnostic architecture has been split into single-responsibility submodules to maximize modularity and performance:
    * `zoom_pip_ui.cpp`: Main view layout, context rendering, and FatBits grid magnifying lens.
    * `zoom_pip_metrics.cpp`: Vector signal filters calculating real-time **RMS Voltage**, **Peak-to-Peak (Vpp)** swing boundaries, and **Zero-Crossing Rate (ZCR)** spectral frequencies.
    * `zoom_pip_timers.cpp`: High-resolution state stopwatches tracking precise millisecond phase-settle intervals.
* **Predictive Momentum Flags:** Reusable macro structures appending real-time directional trend symbols (`^` Upward climb, `v` Downward drop, `-` Stable plateau) to track how field amplitudes alter relative to target limits ahead of satisfying the gate.
* **Global Consensus Truth Gating:** The master logic track behaves as a strict consensus validator, rendering a neutral gray `[-]` during transitional phase-switching noise and only snapping to a firm `[T]` or `[F]` once all parallel metrics lock into synchronous agreement.
* **Phase-Locked Emitter Coherence:** Deactivated or muted independent laser channels continue accumulating phase clock vector steps concurrently behind the scenes. This guarantees absolute temporal coherence and clean, balanced destructive interference the exact frame a checkbox is re-enabled.
* **Compressed PNG Asset Engine:** Stripped legacy uncompressed 1.4MB BMP dependencies down to a clean, fast `libsdl2-image` PNG pipeline loading and saving file templates at a featherweight storage footprint.

---

## 🛠️ Workstation Layout Matrix

The screen geometry is split into three highly optimized workspace areas:

1.  **Global Configuration Suite (Left Column - 240px wide):** Controls real-time simulation presets, procedural lab assets toolkits, custom image map loaders, and high-dynamic-range 32-bit OpenEXR frame data exporters.
2.  **Simulation Monitor Canvas (Top Right - 870px x 485px):** Renders raw amplitude phase ripples or steady-state intensity focus patterns using hardware-bound OpenGL texture arrays. Allows real-time interactive probe tracking and source node drag-placement.
3.  **Analysis Footer Workspace (Bottom Right - 870px x 210px):** Integrates the FatBits zoom PiP window, coordinate adjustments, tabular verification benches, settle timers, and color-coded dual-channel oscilloscopes (Cyan for Voltage, Amber for Frequency).

---

## 🛠️ Build & Compilation Instructions

### Prerequisites
Install standard C++ compilation systems, CMake, SDL2, OpenGL, and the required compressed image decoding frameworks on your Ubuntu machine:
```bash
sudo apt update
sudo apt install build-essential cmake libsdl2-dev libsdl2-image-dev libgl1-mesa-dev
```

### Automated Start Script
The repository includes an optimized compilation and runtime automation initialization script. Execute it directly from your terminal root path to build and boot the studio instantly:
```bash
./run.sh
```

### Manual Compilation Sequence
If you prefer to step-build the project target matrices manually, use the standard CMake sequence:
```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j\$(nproc)
cd ..
./build/WaveGuideNative
```

---

## 📄 Credits, Lineage and Licensing
This project is free open-source software distributed under the permissive **GPL License**—see the accompanying `LICENSE` file for full licensing text details.

WaveGuide-Native is proudly inspired by, derived from, and built on top of the exceptional work of the following creators:

* **Jeroen Vleggaar ([Huygens Optics](https://www.youtube.com/@HuygensOptics))** - Author of the original *Wave Dude 2D* software concept which pioneered semantic 24-bit color channel map simulations.
* **0x23 (Diffraction Limited)** - Author of the high-speed GPU-accelerated [*WaveSimulator2D*](https://github.com/0x23/WaveSimulator2D) workspace.
* **Alexander Gräf** - Author of the advanced C# interactive design program [*WaveDudeNext*](https://github.com/graealex/WaveDudeNext).
* **Omar Cornut** - Mastermind behind the bloat-free [*Dear ImGui*](https://github.com/ocornut/imgui/) immediate-mode user interface paradigm framework.

