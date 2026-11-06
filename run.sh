cd ~/Documents/dev/c/WaveGuide-Native/build
rm -rf *
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
./WaveGuideNative

