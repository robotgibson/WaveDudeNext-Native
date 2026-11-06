#include "engine/WaveEngine.hpp"
#include "engine/fdtd_stencils.h"
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

    // Use boundary contraction limits (y=2, x=2) to prevent out-of-bounds stencil evaluation
    #ifdef HAS_OPENMP
    #pragma omp parallel for schedule(static)
    #endif
    for (int y = 2; y < height - 2; ++y) {
        for (int x = 2; x < width - 2; ++x) {
            int idx = y * width + x;
            float laplacian = computeSpatialLaplacian(curr, idx, width, activeStencilMode);
            float waveSpeed = 0.45f / nMap[idx]; 
            float sigma = sigmaMap[idx];
            next[idx] = (2.0f * curr[idx] - prev[idx] * (1.0f - sigma) + (waveSpeed * waveSpeed * laplacian)) / (1.0f + sigma);
        }
    }

    int gX = globalOriginX;
    int gY = globalOriginY;

    for (size_t i = 0; i < customSourceNodes.size(); ++i) {
        auto& node = customSourceNodes[i];
        if (!node.isActive) continue;

        node.phaseValue += node.frequencyOverride;
        
        int absoluteX = std::clamp(gX + node.offsetX, 2, width - 3);
        int absoluteY = std::clamp(gY + node.offsetY, 2, height - 3);
        int gridIdx = absoluteY * width + absoluteX;
        
        float radiansOffset = node.phaseOffsetDeg * (3.14159265f / 180.0f);

        if (activeSceneMode == 0) { // AND Gate
            if (i == 0)      next[gridIdx] = inputA_Active ? std::sin(node.phaseValue) * (globalAmplitude * node.amplitudeScale) : 0.0f;
            else if (i == 1) next[gridIdx] = std::sin(node.phaseValue + radiansOffset) * (globalAmplitude * node.amplitudeScale); 
            else if (i == 2) next[gridIdx] = inputB_Active ? std::sin(node.phaseValue) * (globalAmplitude * node.amplitudeScale) : 0.0f;
            else             next[gridIdx] = std::sin(node.phaseValue + radiansOffset) * (globalAmplitude * node.amplitudeScale); // FIXED: Fallback lets added sources emit waves
        } 
        else if (activeSceneMode == 4) { // NOT Gate
            if (i == 0)      next[gridIdx] = inputA_Active ? std::sin(node.phaseValue) * (globalAmplitude * node.amplitudeScale) : 0.0f;
            else if (i == 1) next[gridIdx] = std::sin(node.phaseValue + radiansOffset) * (globalAmplitude * node.amplitudeScale); 
            else             next[gridIdx] = std::sin(node.phaseValue + radiansOffset) * (globalAmplitude * node.amplitudeScale); // FIXED: Fallback lets added sources emit waves
        }
        else {
            next[gridIdx] = std::sin(node.phaseValue + radiansOffset) * (globalAmplitude * node.amplitudeScale);
        }
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
            b = static_cast<uint8_t>(std::clamp((-val * 240.0f) + 127.0f, 0.0f, 255.0f)); 
            g = static_cast<uint8_t>(std::clamp(r, b, static_cast<uint8_t>(255)) * 0.15f);
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

