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

// FIXED: Implemented a robust 2D rotation matrix solver to render arbitrary prism transformations perfectly
void PrismPart::burnIntoMap(std::vector<float>& indexMap, int width, int height) {
    float rad = rotationDegrees * (3.14159265f / 180.0f);
    float cosA = std::cos(rad);
    float sinA = std::sin(rad);

    // Baseline coordinates of an equilateral triangle centered at origin (0,0)
    float hTri = baseSize * 0.866025f;
    float x0 = -baseSize / 2.0f; float y0 = hTri / 3.0f;
    float x1 = baseSize / 2.0f;  float y1 = hTri / 3.0f;
    float x2 = 0.0f;             float y2 = -2.0f * hTri / 3.0f;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Inverse translation relative to the prism's center position
            float dx = x - posX;
            float dy = y - posY;

            // Apply inverse 2D rotational transformations
            float rotX = dx * cosA + dy * sinA;
            float rotY = -dx * sinA + dy * cosA;

            // Barycentric edge detection checks if transformed coordinates lie inside the bounds
            float d1 = (rotX - x1) * (y0 - y1) - (x0 - x1) * (rotY - y1);
            float d2 = (rotX - x2) * (y1 - y2) - (x1 - x2) * (rotY - y2);
            float d3 = (rotX - x0) * (y2 - y0) - (x2 - x0) * (rotY - y0);

            if (!((d1 < 0 || d2 < 0 || d3 < 0) && (d1 > 0 || d2 > 0 || d3 > 0))) {
                indexMap[y * width + x] = 1.65f; // Set refractive glass index
            }
        }
    }
}
