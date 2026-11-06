#include "engine/WaveEngine.hpp"
#include <cmath>
#include <algorithm>

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
