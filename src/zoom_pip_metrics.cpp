#include "engine/WaveEngine.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

void calculateWaveMetrics(WaveEngine& engine, int pX, int pY, const std::vector<float>& vPlotCache,
                          float& signalRMS, float& measuredFrequency, float& signalPeakMax, float& signalVpp) {
    float sumSquares = 0.0f;
    for (float val : vPlotCache) { sumSquares += val * val; }
    signalRMS = std::sqrt(sumSquares / (vPlotCache.empty() ? 1.0f : vPlotCache.size()));

    int zeroCrossings = 0;
    for (size_t i = 1; i < vPlotCache.size(); ++i) {
        if ((vPlotCache[i - 1] >= 0.0f && vPlotCache[i] < 0.0f) || 
            (vPlotCache[i - 1] < 0.0f && vPlotCache[i] >= 0.0f)) {
            zeroCrossings++;
        }
    }
    measuredFrequency = (signalRMS > 0.01f) ? (zeroCrossings * 3.14159265f) / (vPlotCache.size() * 2.0f) : 0.0f;

    float vMin = vPlotCache.empty() ? 0.0f : *std::min_element(vPlotCache.begin(), vPlotCache.end());
    float vMax = vPlotCache.empty() ? 0.0f : *std::max_element(vPlotCache.begin(), vPlotCache.end());
    signalPeakMax = std::max(std::abs(vMin), std::abs(vMax));
    signalVpp = vMax - vMin;
}

