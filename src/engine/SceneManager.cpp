#include "engine/WaveEngine.hpp"
#include <algorithm>
#include <cmath>

void WaveEngine::switchToAndGate() {
    activeSceneMode = 0; partsList.clear(); setGlobalOrigin(0, 0); clearIndependentSourceNodes();
    
    // FIXED: Position emitters directly inside the centers of the 3 physical guide tracks
    addIndependentSourceNode(280, 284); // Top Track Lane (Input A Channel)
    addIndependentSourceNode(280, 300); // Center Track Lane (Destructive Cancellation Reference)
    addIndependentSourceNode(280, 316); // Bottom Track Lane (Input B Channel)
    
    if (customSourceNodes.size() >= 3) {
        // Configure the center reference pin to be 180 degrees out-of-phase for phase-cancellation logic
        customSourceNodes[1].phaseOffsetDeg = 180.0f; 
        customSourceNodes[1].amplitudeScale = 0.625f; 
    }
    initSimulationSpace(); clearGridFields();
}

void WaveEngine::switchToXorGate() {
    activeSceneMode = 5; partsList.clear(); setGlobalOrigin(0, 0); clearIndependentSourceNodes();
    
    // FIXED: Position XOR split inputs to align smoothly into the top and bottom channel lanes
    addIndependentSourceNode(280, 265); // Diverging Top Input A channel track
    addIndependentSourceNode(280, 335); // Diverging Bottom Input B channel track
    
    initSimulationSpace(); clearGridFields();
}

void WaveEngine::switchToOrGate() {
    activeSceneMode = 3; partsList.clear(); setGlobalOrigin(0, 0); clearIndependentSourceNodes();
    
    // FIXED: Align OR inputs cleanly with the waveguide mouth boundaries
    addIndependentSourceNode(280, 265); // Top Input A
    addIndependentSourceNode(280, 335); // Bottom Input B
    
    if (customSourceNodes.size() >= 2) {
        customSourceNodes[1].phaseOffsetDeg = -30.0f; // Attenuate phase shift to optimize constructive combining
    }
    initSimulationSpace(); clearGridFields();
}

void WaveEngine::switchToNotGate() {
    activeSceneMode = 4; partsList.clear(); setGlobalOrigin(0, 0); clearIndependentSourceNodes();
    
    // FIXED: Align NOT gate control and reference pins directly inside the straight track slots
    addIndependentSourceNode(280, 284); // Top Track Lane (Control Input A)
    addIndependentSourceNode(280, 300); // Center Track Lane (Constant Carrier 1 Reference - Always Active)
    
    if (customSourceNodes.size() >= 2) {
        customSourceNodes[1].phaseOffsetDeg = 180.0f; // Pure out-of-phase inversion mechanism
    }
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
