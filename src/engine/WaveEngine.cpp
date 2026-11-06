#include "engine/WaveEngine.hpp"
#include <algorithm>

WaveEngine::WaveEngine(int w, int h) : width(w), height(h),
    gridCurrent(w * h, 0.0f), gridPrevious(w * h, 0.0f), gridIntensity(w * h, 0.0f),
    refractiveIndexMap(w * h, 1.0f), dampingMap(w * h, 0.0f)
{
    resetToProceduralGates();
}

void WaveEngine::addOpticalPart(std::unique_ptr<OpticalPart> part) {
    partsList.push_back(std::move(part));
}

void WaveEngine::clearGridFields() {
    std::fill(gridCurrent.begin(), gridCurrent.end(), 0.0f);
    std::fill(gridPrevious.begin(), gridPrevious.end(), 0.0f);
    std::fill(gridIntensity.begin(), gridIntensity.end(), 0.0f);
    phaseA = 0.0f; phaseB = 0.0f; phaseRef = 0.0f;
}

void WaveEngine::resetToProceduralGates() {
    activeSceneMode = 0; partsList.clear();
    addOpticalPart(std::make_unique<XorGatePart>("XOR Gate 1", 450, 200));
    addOpticalPart(std::make_unique<AndGatePart>("AND Gate 2", 450, 460));
    sourceX = 280; sourceY1 = 165; sourceY2 = 235;
    initSimulationSpace();
    clearGridFields();
}

bool WaveEngine::loadSceneFromImage(const std::string& filePath) {
    activeSceneMode = 1; partsList.clear();
    int centerX = width / 2 + 100; int centerY = height / 2; int radius = 80;
    sourceX = 60; sourceY1 = height / 2; sourceY2 = height / 2;
    initSimulationSpace();
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int dx = x - centerX; int dy = y - centerY;
            if ((dx * dx + dy * dy) <= (radius * radius)) { refractiveIndexMap[y * width + x] = 1.55f; }
        }
    }
    clearGridFields();
    return true;
}

void WaveEngine::switchToPrismScene() {
    activeSceneMode = 2; partsList.clear();
    addOpticalPart(std::make_unique<PrismPart>("Glass Prism 1", width / 2 + 50, height / 2, 160));
    sourceX = 60; sourceY1 = height / 2 + 60; sourceY2 = height / 2 + 60;
    initSimulationSpace();
    clearGridFields();
}

void WaveEngine::initSimulationSpace() {
    std::fill(refractiveIndexMap.begin(), refractiveIndexMap.end(), 1.0f);
    std::fill(dampingMap.begin(), dampingMap.end(), 0.0f);
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
                dampingMap[y * width + x] = 0.15f * (normDist * normDist); 
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

void WaveEngine::setPhaseShift(float degrees) { phaseOffset = degrees * (3.14159265f / 180.0f); }
void WaveEngine::setFrequency(float freq) { frequency = freq; }
void WaveEngine::setAmplitude(float amp) { sourceAmplitude = amp; }
void WaveEngine::toggleInputs(bool bitA, bool bitB) { inputA_Active = bitA; inputB_Active = bitB; }
float WaveEngine::sampleAmplitude(int x, int y) const { return (x >= 0 && x < width && y >= 0 && y < height) ? gridCurrent[y * width + x] : 0.0f; }
void WaveEngine::setColorPalette(int paletteID) { activeColorPalette = paletteID; }
