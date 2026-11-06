#!/bin/bash
# Move straight to your absolute project root execution context
cd "$(dirname "$0")" || exit 1

# Ensure a clean build directory path matrix structure exists
mkdir -p build && cd build || exit 1

# Force CMake to freshly re-evaluate the GLOB paths, compile, and execute smoothly
cmake -DCMAKE_BUILD_TYPE=Release .. && \
make -j$(nproc) && \
./WaveGuideNative

