# WaveGuide-Native 🌊 (C++ Photonics Studio)

A high-performance, hardware-accelerated **native C++20 port** of the WaveGuide-Native wave physics engine. This application implements a Finite-Difference Time-Domain (FDTD) numerical solver to simulate 2D electromagnetic fields and explore all-optical computing architectures right on Linux.

## 🚀 Key Features

* **Parallelized FDTD Core:** Leverages native OpenMP loop vectorization to distribute 2D Laplacian matrix stencils across all available processor cores concurrently.
* **Multi-Scene Laboratory:** Real-time dynamic switching between automated all-optical logic networks (`XOR` and three-waveguide `AND` couplers), procedural ball lens focus modules, and triangular glass refraction prisms.
* **Live Studio Dashboard:** Interactive sidebar UI powered by Dear ImGui featuring real-time sliders for laser wave power gain, color frequency tuning, phase offsets, execution pause/play states, and multiple render color schemes (including Monochrome Radar Glow).
* **Draggable Oscilloscope Probe:** Real-time click-and-drag crosshair node tool that samples raw field amplitudes anywhere on the canvas and plots rolling waveforms live onto an integrated sidebar chart.

## 🛠️ Build & Compilation Instructions

### Prerequisites
Ensure you have standard C++ build tools, CMake, SDL2, and OpenGL configuration runtimes installed on your Ubuntu system:
```bash
sudo apt update
sudo apt install build-essential cmake libsdl2-dev libgl1-mesa-dev
```

### Compilation Sequence
1. Navigate to the project directory root.
2. Initialize and compile using CMake:
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j\$(nproc)
```

### Running the App
Once compilation finishes successfully, execute the native binary to launch your workbench:
```bash
./WaveGuideNative
```

To force a single-threaded fallback build for profiling benchmarks, you can optionally disable OpenMP packages during the configuration stage:
```bash
cmake -DCMAKE_DISABLE_FIND_PACKAGE_OpenMP=TRUE ..
make -j\$(nproc)
```

## 📄 Credits and license
This project is open-source and distributed under the permissive **GPL License**—see the accompanying `LICENSE` file for full attribution details.

Based on the work of

    Jeroen Vleggaar (Huygens Optics), author of Wave Dude 2D, Copyright (C) 2026: [huygensoptics.com](https://www.youtube.com/@HuygensOptics)
    0x23 (Diffraction Limited), author of WaveSimulator2D: [GitHub](https://github.com/0x23/WaveSimulator2D)

    Alexander Gräf
    [GitHub](https://github.com/graealex/WaveDudeNext)

    Omar Cornut
    [GitHub] (https://github.com/ocornut/imgui/)
