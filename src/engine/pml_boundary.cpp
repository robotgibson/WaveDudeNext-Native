#include "engine/pml_boundary.h"
#include <algorithm>

void initializePMLBoundaryLayers(std::vector<float>& dampingMap, int width, int height, int pmlThickness) {
    dampingMap.assign(width * height, 0.0f);
    
    const float maxSigma = 0.15f; // Max damping factor at the absolute edge matrix limits

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = y * width + x;
            
            // Measure pixel distance relative to the four outer borders
            int distLeft   = x;
            int distRight  = (width - 1) - x;
            int distTop    = y;
            int distBottom = (height - 1) - y;

            int minDist = std::min({distLeft, distRight, distTop, distBottom});

            if (minDist < pmlThickness) {
                // Smooth quadratic ramp: scales seamlessly to prevent secondary step-impedance reflection
                float normalizedDepth = 1.0f - (static_cast<float>(minDist) / pmlThickness);
                dampingMap[idx] = maxSigma * (normalizedDepth * normalizedDepth);
            }
        }
    }
}
