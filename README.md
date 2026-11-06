# WaveGuide-Native 🌊 (C++ Photonics Studio)

A high-performance, hardware-accelerated **native C++20 port** of the WaveGuide wave physics engine framework. This application implements a multi-threaded Finite-Difference Time-Domain (FDTD) numerical solver to simulate 2D electromagnetic fields and explore high-speed all-optical logic architectures right on Linux.

## 🚀 Key Features

* **Parallelized FDTD Math Core:** Leverages OpenMP loop vectorization and explicit `__restrict` pointers to distribute 2D Laplacian matrix calculation stencils across all available CPU thread cores concurrently.
* **Granular Multi-Scene Matrix:** Seamless real-time layout profile toggles across individual standalone all-optical logic gates (`AND` couplers, `OR` splitters, `NOT` inverters, and physical `XOR` destructive phase-cancellation junctions), procedural ball lenses, and variable glass prisms.
* **Global Cluster Transformations:** Integrated master translation coordinate sliders (`Global Origin X/Y`) to shift all active emitters at once as a single structural unit, while individual sources track positions as relative offsets.
* **Precision Step Controls:** Reusable template-macro rows appending navigation arrow tags (`<` and `>`) next to every single configuration property slider for exact `-1` and `+1` increment fine-tuning.
* **Dual-Channel Logic Analyzer:** Advanced real-time diagnostic bench plotting rolling waveforms and evaluating dual parallel output states simultaneously: **Voltage Logic (V)** via root-mean-square (RMS) energy states, and **Spectral Logic (F)** via Zero-Crossing Rate (ZCR) color match locks.
* **Compressed PNG Asset Engine:** Stripped legacy uncompressed 1.4MB BMP dependencies down to a clean, fast `libsdl2-image` PNG pipeline loading and saving file templates at a featherweight storage footprint.

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

## 📄 Credits, Lineage and Licensing
This project is free open-source software distributed under the permissive **GPL License**—see the accompanying `LICENSE` file for full licensing text details.

WaveGuide-Native is proudly inspired by, derived from, and built on top of the exceptional work of the following creators:

* **Jeroen Vleggaar ([Huygens Optics](https://www.youtube.com/@HuygensOptics))** - Author of the original *Wave Dude 2D* software concept which pioneered semantic 24-bit color channel map simulations.
* **0x23 (Diffraction Limited)** - Author of the high-speed GPU-accelerated [*WaveSimulator2D*](https://github.com/0x23/WaveSimulator2D) workspace.
* **Alexander Gräf** - Author of the advanced C# interactive design program [*WaveDudeNext*](https://github.com/graealex/WaveDudeNext).
* **Omar Cornut** - Mastermind behind the bloat-free [*Dear ImGui*](https://github.com/ocornut/imgui/) immediate-mode user interface paradigm framework.
