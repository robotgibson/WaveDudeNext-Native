#include "engine/WaveEngine.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

void XorGatePart::burnIntoMap(std::vector<float>& indexMap, int width, int height) {
    int gateLength = 340; int halfLen = gateLength / 2; int fiberWidth = 10;  
    int localStart = posX - halfLen; int localEnd = posX + halfLen; int narrowGap = 10; int wideGap = 35; 
    for (int x = std::max(0, localStart); x < std::min(width, localEnd); ++x) {
        float t = static_cast<float>(x - localStart) / gateLength;
        int targetA_Y = posY - wideGap; int targetB_Y = posY + wideGap;
        if (t < 0.35f) { 
            float normT = t / 0.35f; float smoothT = normT * normT * (3.0f - 2.0f * normT);
            targetA_Y = posY - wideGap + (wideGap - narrowGap) * smoothT;
            targetB_Y = posY + wideGap - (wideGap - narrowGap) * smoothT;
        } else if (t >= 0.35f && t <= 0.65f) { 
            targetA_Y = posY - narrowGap; targetB_Y = posY + narrowGap;
        } else { 
            float normT = (t - 0.65f) / 0.35f; float smoothT = normT * normT * (3.0f - 2.0f * normT);
            targetA_Y = posY - narrowGap - (wideGap - narrowGap) * smoothT;
            targetB_Y = posY + narrowGap + (wideGap - narrowGap) * smoothT;
        }
        for (int y = 0; y < height; ++y) {
            if ((y >= targetA_Y - fiberWidth/2 && y <= targetA_Y + fiberWidth/2) ||
                (y >= targetB_Y - fiberWidth/2 && y <= targetB_Y + fiberWidth/2)) {
                if (y >= 0 && y < height) indexMap[y * width + x] = 1.7f;
            }
        }
    }
}

void AndGatePart::burnIntoMap(std::vector<float>& indexMap, int width, int height) {
    int gateLength = 340; int halfLen = gateLength / 2; int fiberWidth = 8;
    int localStart = posX - halfLen; int localEnd = posX + halfLen; int gap = 16; 
    for (int x = std::max(0, localStart); x < std::min(width, localEnd); ++x) {
        int trackTop = posY - gap; int trackCenter = posY; int trackBottom = posY + gap;
        for (int y = 0; y < height; ++y) {
            if ((y >= trackTop - fiberWidth/2 && y <= trackTop + fiberWidth/2) ||
                (y >= trackCenter - fiberWidth/2 && y <= trackCenter + fiberWidth/2) ||
                (y >= trackBottom - fiberWidth/2 && y <= trackBottom + fiberWidth/2)) {
                if (y >= 0 && y < height) indexMap[y * width + x] = 1.7f;
            }
        }
    }
}

void PrismPart::burnIntoMap(std::vector<float>& indexMap, int width, int height) {
    int x0 = posX - baseSize / 2; int y0 = posY + baseSize / 2;
    int x1 = posX + baseSize / 2; int y1 = posY + baseSize / 2;
    int x2 = posX;                int y2 = posY - (int)(baseSize * 0.866f) / 2;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float d1 = (x - x1) * (y0 - y1) - (x0 - x1) * (y - y1);
            float d2 = (x - x2) * (y1 - y2) - (x1 - x2) * (y - y2);
            float d3 = (x - x0) * (y2 - y0) - (x2 - x0) * (y - y0);
            if (!((d1 < 0 || d2 < 0 || d3 < 0) && (d1 > 0 || d2 > 0 || d3 > 0))) {
                if (y >= 0 && y < height && x >= 0 && x < width) indexMap[y * width + x] = 1.65f;
            }
        }
    }
}

WaveEngine::WaveEngine(int w, int h) : width(w), height(h),
    gridCurrent(w * h, 0.0f), gridPrevious(w * h, 0.0f),
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
}

void WaveEngine::setPhaseShift(float degrees) { phaseOffset = degrees * (3.14159265f / 180.0f); }
void WaveEngine::setFrequency(float freq) { frequency = freq; }
void WaveEngine::setAmplitude(float amp) { sourceAmplitude = amp; }
void WaveEngine::toggleInputs(bool bitA, bool bitB) { inputA_Active = bitA; inputB_Active = bitB; }
float WaveEngine::sampleAmplitude(int x, int y) const { return (x >= 0 && x < width && y >= 0 && y < height) ? gridCurrent[y * width + x] : 0.0f; }
void WaveEngine::setColorPalette(int paletteID) { activeColorPalette = paletteID; }

void WaveEngine::updatePixels(uint32_t* pixelBuffer) {
    #ifdef HAS_OPENMP
    #pragma omp parallel for schedule(static)
    #endif
    for (int i = 0; i < width * height; i++) {
        float waveAmp = gridCurrent[i]; uint8_t r = 0, g = 0, b = 0;
        if (activeColorPalette == 0) {
            r = static_cast<uint8_t>(std::clamp((waveAmp * 240.0f) + 127.0f, 0.0f, 255.0f));
            b = static_cast<uint8_t>(std::clamp((-waveAmp * 240.0f) + 127.0f, 0.0f, 255.0f)); g = static_cast<uint8_t>(std::fmin(r, b) * 0.2f);
        } else if (activeColorPalette == 1) {
            g = static_cast<uint8_t>(std::clamp((waveAmp * 240.0f) + 127.0f, 0.0f, 255.0f));
            r = static_cast<uint8_t>(std::clamp((-waveAmp * 240.0f) + 127.0f, 0.0f, 255.0f)); b = r;
        } else {
            float absAmp = std::fabs(waveAmp) * 2.5f; g = static_cast<uint8_t>(std::clamp(absAmp * 255.0f, 0.0f, 255.0f)); r = b = static_cast<uint8_t>(g * 0.1f);
        }
        if (refractiveIndexMap[i] > 1.1f) { if (activeColorPalette == 1) b = std::clamp(b + 40, 0, 255); else g = std::clamp(g + 35, 0, 255); }
        if (dampingMap[i] > 0.4f && refractiveIndexMap[i] < 1.1f) { r = std::clamp(r - 20, 0, 255); b = std::clamp(b - 20, 0, 255); g = std::clamp(g - 20, 0, 255); }
        pixelBuffer[i] = (255 << 24) | (r << 16) | (g << 8) | b;
    }
}
