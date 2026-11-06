#!/bin/bash
cd ~/Documents/dev/c/WaveGuide-Native/build || exit 1

# Run configuration, compilation, and execution as a strict chained pipeline
cmake -DCMAKE_BUILD_TYPE=Release .. && \
make -j$(nproc) && \
./WaveGuideNative
