#include "engine/WaveEngine.hpp"
#include <algorithm>
#include <cmath>

void WaveEngine::resetToProceduralGates() {
    activeSceneMode = 0; 
    partsList.clear();
    setGlobalOrigin(0, 0); 
    clearIndependentSourceNodes();
    
    addIndependentSourceNode(280, 165); 
    addIndependentSourceNode(280, 235); 
    addIndependentSourceNode(280, 444); 
    addIndependentSourceNode(280, 460); 
    addIndependentSourceNode(280, 476); 
    
    if (customSourceNodes.size() >= 4) {
        customSourceNodes[3].phaseOffsetDeg = 180.0f; 
        customSourceNodes[3].amplitudeScale = 0.625f; 
    }

    initSimulationSpace();
    clearGridFields();
}

void WaveEngine::switchToPrismScene() {
    activeSceneMode = 2; 
    partsList.clear();
    setGlobalOrigin(0, 0);
    clearIndependentSourceNodes();
    
    addIndependentSourceNode(60, height / 2 + 60); 
    addOpticalPart(std::make_unique<PrismPart>("Glass Prism 1", 450, 300, 160, 0.0f));
    
    initSimulationSpace();
    clearGridFields();
}

void WaveEngine::initSimulationSpace() {
    std::fill(refractiveIndexMap.begin(), refractiveIndexMap.end(), 1.0f);
    std::fill(dampingMap.begin(), dampingMap.end(), 0.0f);
    
    if (partsList.empty()) {
        if (activeSceneMode == 0) {
            addOpticalPart(std::make_unique<XorGatePart>("XOR Gate 1", 450, 200));
            addOpticalPart(std::make_unique<AndGatePart>("AND Gate 2", 450, 460));
        } else if (activeSceneMode == 2) {
            addOpticalPart(std::make_unique<PrismPart>("Glass Prism 1", 450, 300, 160, 0.0f));
        }
    }

    for (const auto& part : partsList) { part->burnIntoMap(refractiveIndexMap, width, height); }
    
    int border = 40;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int dist = 0;
            if (x < border) dist = std::max(dist, border - x);
            if (x > width - border) dist = std::max(dist, x - (width - border));
            if (y < border) dist = std::max(dist, border - y);
            if (y > height - border) dist = std::max(dist, y - (height - border));
            if (dist > 0) {
                float normDist = static_cast<float>(dist) / border;
                dampingMap[y * width + x] = std::max(dampingMap[y * width + x], 0.15f * (normDist * normDist)); 
            }
        }
    }
    if (activeSceneMode == 0) {
        int barrierX = 273; 
        for (int y = 50; y < 550; ++y) {
            for (int x = barrierX - 15; x <= barrierX; ++x) { dampingMap[y * width + x] = 0.8f; }
        }
    }
}
