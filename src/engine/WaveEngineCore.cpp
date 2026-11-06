#include "engine/WaveEngine.hpp"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

WaveEngine::WaveEngine(int w, int h) : width(w), height(h),
    gridCurrent(w * h, 0.0f), gridPrevious(w * h, 0.0f), gridIntensity(w * h, 0.0f),
    refractiveIndexMap(w * h, 1.0f), dampingMap(w * h, 0.0f)
{
    IMG_Init(IMG_INIT_PNG);
    resetToProceduralGates();
}

void WaveEngine::addOpticalPart(std::unique_ptr<OpticalPart> part) {
    partsList.push_back(std::move(part));
}

void WaveEngine::clearGridFields() {
    std::fill(gridCurrent.begin(), gridCurrent.end(), 0.0f);
    std::fill(gridPrevious.begin(), gridPrevious.end(), 0.0f);
    std::fill(gridIntensity.begin(), gridIntensity.end(), 0.0f);
    for (auto& node : customSourceNodes) { node.phaseValue = 0.0f; }
}

void WaveEngine::addIndependentSourceNode(int absoluteX, int absoluteY) {
    int cx = std::clamp(absoluteX, 5, width - 5);
    int cy = std::clamp(absoluteY, 5, height - 5);
    
    for (const auto& node : customSourceNodes) {
        int absNodeX = globalOriginX + node.offsetX;
        int absNodeY = globalOriginY + node.offsetY;
        if (std::abs(absNodeX - cx) < 4 && std::abs(absNodeY - cy) < 4) return;
    }

    if (customSourceNodes.size() < 16) {
        CustomLaserNode node;
        node.offsetX = cx - globalOriginX; 
        node.offsetY = cy - globalOriginY;
        node.frequencyOverride = globalFrequency;
        node.amplitudeScale = 1.0f;
        node.isActive = true;
        customSourceNodes.push_back(node);
        selectedNodeIndex = static_cast<int>(customSourceNodes.size()) - 1; 
    }
}

void WaveEngine::clearIndependentSourceNodes() {
    customSourceNodes.clear(); 
    selectedNodeIndex = -1;
}
