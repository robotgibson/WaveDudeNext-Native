#include "engine/WaveEngine.hpp"
#include <cmath>
#include <algorithm>

// --- XOR Gate Waveguide Burn Architecture ---
void XorGatePart::burnIntoMap(std::vector<float>& indexMap, int width, int height) {
    int gateLength = 240; 
    int halfLen = gateLength / 2;
    int fiberWidth = 8;
    
    int localStart = posX - halfLen;
    int localEnd = posX + halfLen;
    int narrowGap = 12;
    int wideGap = 30; 

    for (int x = std::max(0, localStart); x < std::min(width, localEnd); ++x) {
        float t = static_cast<float>(x - localStart) / gateLength;
        
        int targetA_Y = posY - wideGap;
        int targetB_Y = posY + wideGap;

        if (t < 0.35f) { 
            float normT = t / 0.35f;
            float smoothT = normT * normT * (3.0f - 2.0f * normT);
            targetA_Y = posY - wideGap + (wideGap - narrowGap) * smoothT;
            targetB_Y = posY + wideGap - (wideGap - narrowGap) * smoothT;
        } 
        else if (t >= 0.35f && t <= 0.65f) { 
            targetA_Y = posY - narrowGap;
            targetB_Y = posY + narrowGap;
        } 
        else { 
            float normT = (t - 0.65f) / 0.35f;
            float smoothT = normT * normT * (3.0f - 2.0f * normT);
            targetA_Y = posY - narrowGap - (wideGap - narrowGap) * smoothT;
            targetB_Y = posY + narrowGap + (wideGap - narrowGap) * smoothT;
        }

        for (int y = 0; y < height; ++y) {
            if ((y >= targetA_Y - fiberWidth/2 && y <= targetA_Y + fiberWidth/2) ||
                (y >= targetB_Y - fiberWidth/2 && y <= targetB_Y + fiberWidth/2)) {
                if (y >= 0 && y < height) {
                    indexMap[y * width + x] = 1.8f;
                }
            }
        }
    }
}

// --- AND Gate Waveguide Burn Architecture ---
void AndGatePart::burnIntoMap(std::vector<float>& indexMap, int width, int height) {
    int gateLength = 240;
    int halfLen = gateLength / 2;
    int localStart = posX - halfLen;
    int localEnd = posX + halfLen;

    for (int x = std::max(0, localStart); x < std::min(width, localEnd); ++x) {
        for (int y = posY - 4; y <= posY + 4; ++y) {
            if (y >= 0 && y < height) {
                indexMap[y * width + x] = 1.8f; 
            }
        }
    }
}

// --- WaveEngine Main Pipelines ---
WaveEngine::WaveEngine(int w, int h) : width(w), height(h),
    gridCurrent(w * h, 0.0f), gridPrevious(w * h, 0.0f),
    refractiveIndexMap(w * h, 1.4f), dampingMap(w * h, 0.0f)
{
    addOpticalPart(std::make_unique<XorGatePart>("XOR Gate 1", 400, 200));
    addOpticalPart(std::make_unique<AndGatePart>("AND Gate 2", 400, 420));

    // Adjusted to a smoother operational wavelength frequency
    frequency = 0.25f; 

    initSimulationSpace();
}

void WaveEngine::addOpticalPart(std::unique_ptr<OpticalPart> part) {
    partsList.push_back(std::move(part));
}

void WaveEngine::initSimulationSpace() {
    std::fill(refractiveIndexMap.begin(), refractiveIndexMap.end(), 1.4f);
    std::fill(dampingMap.begin(), dampingMap.end(), 0.0f);

    for (const auto& part : partsList) {
        part->burnIntoMap(refractiveIndexMap, width, height);
    }

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
                dampingMap[y * width + x] = 0.1f * (normDist * normDist); 
            }
        }
    }
}

void WaveEngine::stepSimulation() {
    // Create an explicit separate grid buffer to avoid dirty in-place reads
    std::vector<float> gridNext(width * height, 0.0f);

    float* __restrict next = gridNext.data();
    const float* __restrict curr = gridCurrent.data();
    const float* __restrict prev = gridPrevious.data();
    const float* __restrict nMap = refractiveIndexMap.data();
    const float* __restrict sigmaMap = dampingMap.data();

    int inputX = 285;
    int idxSourceA = (200 - 30) * width + inputX;
    int idxSourceB = (200 + 30) * width + inputX;

    phaseA += frequency;
    phaseB += frequency;

    // 1. Compute the wave propagation into the next time step array
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int idx = y * width + x;
            
            float laplacian = curr[idx + 1] + curr[idx - 1] +
                              curr[idx + width] + curr[idx - width] - 
                              4.0f * curr[idx];

            float waveSpeed = 0.35f / nMap[idx]; 
            float sigma = sigmaMap[idx];

            next[idx] = (2.0f * curr[idx] - prev[idx] * (1.0f - sigma) + (waveSpeed * waveSpeed * laplacian)) / (1.0f + sigma);
        }
    }

    // 2. Lock continuous wave signals strictly onto the driving coordinates
    next[idxSourceA] = std::sin(phaseA) * 0.7f;
    next[idxSourceB] = std::sin(phaseB + phaseOffset) * 0.7f;

    // 3. Perform a safe, clean double-buffer assignment step
    gridPrevious = gridCurrent;
    gridCurrent = gridNext;
}

void WaveEngine::setPhaseShift(float degrees) {
    phaseOffset = degrees * (3.14159265f / 180.0f);
}

void WaveEngine::updatePixels(uint32_t* pixelBuffer) {
    for (int i = 0; i < width * height; ++i) {
        float waveAmp = gridCurrent[i];
        
        uint8_t r = static_cast<uint8_t>(std::clamp((waveAmp * 160.0f) + 127.0f, 0.0f, 255.0f));
        uint8_t b = static_cast<uint8_t>(std::clamp((-waveAmp * 160.0f) + 127.0f, 0.0f, 255.0f));
        uint8_t g = static_cast<uint8_t>(std::fmin(r, b) * 0.2f);

        if (refractiveIndexMap[i] > 1.5f) {
            g = std::clamp(g + 35, 0, 255);
        }

        pixelBuffer[i] = (255 << 24) | (r << 16) | (g << 8) | b;
    }
}
