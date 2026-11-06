#pragma once
#include "engine/WaveEngine.hpp"

// Computes the position-dependent 2D spatial discrete Laplacian profile based on active order precision
inline float computeSpatialLaplacian(const float* __restrict curr, int idx, int width, WaveEngine::StencilPrecision mode) {
    if (mode == WaveEngine::StencilPrecision::FourthOrder) {
        // Higher-Order 4th Spatial Stencil Coefficients Matrix Weights (-1/12, 4/3, -5)
        float lapX = (-curr[idx + 2] + 16.0f * curr[idx + 1] + 16.0f * curr[idx - 1] - curr[idx - 2]) / 12.0f;
        float lapY = (-curr[idx + 2 * width] + 16.0f * curr[idx + width] + 16.0f * curr[idx - width] - curr[idx - 2 * width]) / 12.0f;
        return lapX + lapY - 5.0f * curr[idx];
    }
    // Legacy standard 2-point central difference stencil fallback
    return curr[idx + 1] + curr[idx - 1] + curr[idx + width] + curr[idx - width] - 4.0f * curr[idx];
}

