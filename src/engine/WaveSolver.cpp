#include "engine/WaveEngine.hpp"
#include <cmath>
#include <algorithm>

void WaveEngine::stepSimulation() {
    if (isPaused) return;

    std::vector<float> gridNext(width * height, 0.0f);
    float* __restrict next = gridNext.data();
    const float* __restrict curr = gridCurrent.data();
    const float* __restrict prev = gridPrevious.data();
    const float* __restrict nMap = refractiveIndexMap.data();
    const float* __restrict sigmaMap = dampingMap.data();

    #ifdef HAS_OPENMP
    #pragma omp parallel for schedule(static)
    #endif
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int idx = y * width + x;
            float laplacian = curr[idx + 1] + curr[idx - 1] + curr[idx + width] + curr[idx - width] - 4.0f * curr[idx];
            float waveSpeed = 0.45f / nMap[idx]; float sigma = sigmaMap[idx];
            next[idx] = (2.0f * curr[idx] - prev[idx] * (1.0f - sigma) + (waveSpeed * waveSpeed * laplacian)) / (1.0f + sigma);
        }
    }

    phaseA += frequency; phaseB += frequency; phaseRef += frequency; 
    if (activeSceneMode == 0) {
        next[sourceY1 * width + sourceX] = std::sin(phaseA) * sourceAmplitude;
        next[sourceY2 * width + sourceX] = std::sin(phaseB + phaseOffset) * sourceAmplitude;
        int idxAndTop = (460 - 16) * width + sourceX; int idxAndCenter = (460) * width + sourceX; int idxAndBottom = (460 + 16) * width + sourceX;
        next[idxAndTop] = inputA_Active ? std::sin(phaseA) * sourceAmplitude : 0.0f;
        next[idxAndCenter] = std::sin(phaseRef + 3.14159265f) * (sourceAmplitude * 0.625f); 
        next[idxAndBottom] = inputB_Active ? std::sin(phaseB) * sourceAmplitude : 0.0f;
    } else {
        next[sourceY1 * width + sourceX] = std::sin(phaseA) * sourceAmplitude;
    }
    gridPrevious = gridCurrent;
    gridCurrent = gridNext;

    float* __restrict intensity = gridIntensity.data();
    const float* __restrict currentWave = gridCurrent.data();
    #ifdef HAS_OPENMP
    #pragma omp parallel for schedule(static)
    #endif
    for (int i = 0; i < width * height; ++i) {
        float instantPower = currentWave[i] * currentWave[i];
        intensity[i] = (intensity[i] * 0.96f) + (instantPower * 0.04f);
    }
}

void WaveEngine::updatePixels(uint32_t* pixelBuffer) {
    const float* __restrict renderSource = showIntensityMode ? gridIntensity.data() : gridCurrent.data();

    #ifdef HAS_OPENMP
    #pragma omp parallel for schedule(static)
    #endif
    for (int i = 0; i < width * height; i++) {
        float val = renderSource[i]; uint8_t r = 0, g = 0, b = 0;
        
        if (!showIntensityMode) {
            r = static_cast<uint8_t>(std::clamp((val * 240.0f) + 127.0f, 0.0f, 255.0f));
            b = static_cast<uint8_t>(std::clamp((-val * 240.0f) + 127.0f, 0.0f, 255.0f)); g = static_cast<uint8_t>(std::fmin(r, b) * 0.2f);
        } else {
            float scaledIntensity = val * 8.0f; 
            if (activeColorPalette == 0) {
                r = static_cast<uint8_t>(std::clamp(scaledIntensity * 255.0f, 0.0f, 255.0f));
                g = static_cast<uint8_t>(std::clamp(scaledIntensity * 130.0f, 0.0f, 255.0f)); b = static_cast<uint8_t>(std::clamp(scaledIntensity * 20.0f, 0.0f, 255.0f));
            } else if (activeColorPalette == 1) {
                r = b = static_cast<uint8_t>(std::clamp(scaledIntensity * 255.0f, 0.0f, 255.0f)); g = static_cast<uint8_t>(scaledIntensity * 30.0f);
            } else {
                g = static_cast<uint8_t>(std::clamp(scaledIntensity * 255.0f, 0.0f, 255.0f)); r = b = static_cast<uint8_t>(g * 0.1f);
            }
        }
        
        if (refractiveIndexMap[i] > 1.1f) { if (activeColorPalette == 1) b = std::clamp(b + 40, 0, 255); else g = std::clamp(g + 35, 0, 255); }
        if (dampingMap[i] > 0.4f && refractiveIndexMap[i] < 1.1f) { r = std::clamp(r - 20, 0, 255); b = std::clamp(b - 20, 0, 255); g = std::clamp(g - 20, 0, 255); }
        pixelBuffer[i] = (255 << 24) | (r << 16) | (g << 8) | b;
    }
}
