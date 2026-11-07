#include "engine/WaveEngine.hpp"
#include "engine/scene_loader.hpp"
#include <algorithm>
#include <cmath>

void WaveEngine::switchToAndGate() {
    activeSceneMode = 0; partsList.clear(); setGlobalOrigin(0, 0);
    loadSceneProfileFromJSON(*this, 0);
    initSimulationSpace(); clearGridFields();
}

void WaveEngine::switchToXorGate() {
    activeSceneMode = 5; partsList.clear(); setGlobalOrigin(0, 0);
    loadSceneProfileFromJSON(*this, 5);
    initSimulationSpace(); clearGridFields();
}

void WaveEngine::switchToOrGate() {
    activeSceneMode = 3; partsList.clear(); setGlobalOrigin(0, 0);
    loadSceneProfileFromJSON(*this, 3);
    initSimulationSpace(); clearGridFields();
}

void WaveEngine::switchToNotGate() {
    activeSceneMode = 4; partsList.clear(); setGlobalOrigin(0, 0);
    loadSceneProfileFromJSON(*this, 4);
    initSimulationSpace(); clearGridFields();
}

void WaveEngine::switchToPrismScene() {
    activeSceneMode = 2; partsList.clear(); setGlobalOrigin(0, 0); clearIndependentSourceNodes();
    addIndependentSourceNode(60, height / 2 + 60); 
    initSimulationSpace(); clearGridFields();
}

void WaveEngine::initSimulationSpace() {
    std::fill(refractiveIndexMap.begin(), refractiveIndexMap.end(), 1.0f);
    std::fill(dampingMap.begin(), dampingMap.end(), 0.0f);
    
    if (partsList.empty()) {
        if (activeSceneMode == 0 || activeSceneMode == 4) {
            addOpticalPart(std::make_unique<AndGatePart>("AND/NOT Straight Waveguide Layout", 450, 300));
        } else if (activeSceneMode == 3) {
            addOpticalPart(std::make_unique<XorGatePart>("OR Splitter Waveguide Layout", 450, 300));
        } else if (activeSceneMode == 5) {
            addOpticalPart(std::make_unique<XorGatePart>("XOR Converging Layout", 450, 300));
            addOpticalPart(std::make_unique<PrismPart>("XOR Phase Cancellation Block", 445, 300, 24, 90.0f));
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
    
    if (activeSceneMode == 0 || activeSceneMode == 3 || activeSceneMode == 4 || activeSceneMode == 5) {
        int barrierX = 273; 
        for (int y = 50; y < 550; ++y) {
            for (int x = barrierX - 15; x <= barrierX; ++x) { dampingMap[y * width + x] = 0.8f; }
        }
    }
}

