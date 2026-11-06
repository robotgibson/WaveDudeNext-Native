#include "exr_exporter_ui.h"  // <--- Include the new split header file
#include "imgui.h"
#include "engine/WaveEngine.hpp"
#include "UIRegistry.hpp"
#include <vector>
#include <cstring>
#include <cmath>
#include <numeric>

void renderPrismSettings(WaveEngine& engine);
void renderEmitterMatrix(WaveEngine& engine, float& sliderAmp, float& sliderFreq, bool& bitA, bool& bitB, int& pX, int& pY);

void renderTuningDesk(WaveEngine& engine, char* pathBuffer, size_t bufferSize, 
                      float& sliderAmp, float& sliderFreq, float& sliderPhase,
                      bool& bitA, bool& bitB, int& paletteIdx, int& pX, int& pY) {
    
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(300, 720), ImGuiCond_Always);
    ImGui::Begin("Wave Tuning Desk", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    
    ImGui::Text("Simulation Scene Profile:");
    if (ImGui::Button("AND Gate Circuit", ImVec2(134, 24))) {
        engine.switchToAndGate(); sliderFreq = 0.08f; engine.setGlobalFrequency(0.08f); pX = 620; pY = 300;
        engine.setSelectedNodeIndex(-1);
    }
    ImGui::SameLine();
    if (ImGui::Button("OR Gate Circuit", ImVec2(134, 24))) {
        engine.switchToOrGate(); sliderFreq = 0.08f; engine.setGlobalFrequency(0.08f); pX = 620; pY = 300;
        engine.setSelectedNodeIndex(-1);
    }
    if (ImGui::Button("NOT Gate Circuit", ImVec2(134, 24))) {
        engine.switchToNotGate(); sliderFreq = 0.08f; engine.setGlobalFrequency(0.08f); pX = 620; pY = 300;
        engine.setSelectedNodeIndex(-1);
    }
    ImGui::SameLine();
    if (ImGui::Button("XOR Gate Circuit", ImVec2(134, 24))) {
        engine.switchToXorGate(); sliderFreq = 0.08f; engine.setGlobalFrequency(0.08f); pX = 620; pY = 300;
        engine.setSelectedNodeIndex(-1);
    }

    if (ImGui::Button("Glass Prism Layout", ImVec2(-1, 26))) {
        engine.switchToPrismScene(); sliderFreq = 0.07f; engine.setGlobalFrequency(0.07f); pX = 550; pY = 380; 
        engine.setSelectedNodeIndex(-1);
    }

    ImGui::Spacing();
    ImGui::Text("Load Map Asset Image (.png):");
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##PathBox", pathBuffer, bufferSize);
    if (ImGui::Button("Compile Custom Scene Canvas Map", ImVec2(-1, 28))) {
        if (engine.loadSceneFromImage(pathBuffer)) {
            pX = 400; pY = 300; engine.setSelectedNodeIndex(-1);
        }
    }

    ImGui::Spacing();
    if (ImGui::CollapsingHeader("Procedural Lab Asset Toolkit", ImGuiTreeNodeFlags_None)) {
        if (ImGui::Button("Load Double Slit Layout", ImVec2(-1, 26))) {
            engine.generateNativeDoubleSlitPNG(); std::strncpy(pathBuffer, "double_slit_experiment.png", bufferSize);
            if (engine.loadSceneFromImage(pathBuffer)) { pX = 320; pY = 300; }
        }
        if (ImGui::Button("Load Spherical Ball Lens", ImVec2(-1, 26))) {
            engine.generateNativeBallLensPNG(); std::strncpy(pathBuffer, "focusing_ball_lens.png", bufferSize);
            if (engine.loadSceneFromImage(pathBuffer)) { pX = 580; pY = 300; }
        }
        if (ImGui::Button("Load Fiber Network Splitter", ImVec2(-1, 26))) {
            engine.generateNativeFiberNetworkPNG(); std::strncpy(pathBuffer, "fiber_splitter_network.png", bufferSize);
            if (engine.loadSceneFromImage(pathBuffer)) { pX = 450; pY = 300; }
        }
    }

    renderPrismSettings(engine);
    renderEmitterMatrix(engine, sliderAmp, sliderFreq, bitA, bitB, pX, pY);

    ImGui::Separator();
    ImGui::Text("Probe Node Diagnostic Analysis:");
    
    static std::vector<float> plotCache(120, 0.0f);
    static float comparisonThreshold = 0.35f;
    static float targetFreqThreshold = 0.080f;
    static float frequencyTolerance = 0.005f;

    float currentSample = engine.sampleAmplitude(pX, pY);
    if (!engine.getPauseState()) {
        plotCache.erase(plotCache.begin());
        plotCache.push_back(currentSample);
    }

    // 1. Calculate Intensity (RMS)
    float sumSquares = 0.0f;
    for (float val : plotCache) { sumSquares += val * val; }
    float signalRMS = std::sqrt(sumSquares / plotCache.size());

    // 2. Analyze Zero-Crossing Rate (ZCR) to calculate frequency
    int zeroCrossings = 0;
    for (size_t i = 1; i < plotCache.size(); ++i) {
        if ((plotCache[i - 1] >= 0.0f && plotCache[i] < 0.0f) || 
            (plotCache[i - 1] < 0.0f && plotCache[i] >= 0.0f)) {
            zeroCrossings++;
        }
    }
    float measuredFrequency = (signalRMS > 0.02f) ? (zeroCrossings * 3.14159265f) / (plotCache.size() * 2.0f) : 0.0f;

    // Render configuration sliders
    PrecisionSlider<float>("Amp Limit", &comparisonThreshold, 0.01f, 1.20f, 0.01f, "Min A = %.2f");
    PrecisionSlider<float>("Target w",   &targetFreqThreshold, 0.03f, 0.18f, 0.001f, "w = %.3f");

    // FIXED: Split logic evaluation markers into separate, parallel metrics outputs
    bool vLogicHigh = (signalRMS >= comparisonThreshold);
    bool fLogicHigh = (signalRMS > 0.02f) && (std::abs(measuredFrequency - targetFreqThreshold) <= frequencyTolerance);

    // 1. Render Voltage/Amplitude Logic State Row
    if (!vLogicHigh) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.3f, 1.0f), "VOLTAGE  LOGIC (V): 0 (LOW)");
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.0f, 1.0f), "VOLTAGE  LOGIC (V): 1 (HIGH)");
    }

    // 2. Render Spectral/Wavelength Logic State Row
    if (!fLogicHigh) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.3f, 1.0f), "SPECTRAL LOGIC (F): 0 (LOW)");
    } else {
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "SPECTRAL LOGIC (F): 1 (HIGH)");
    }
    
    ImGui::TextDisabled("Measured: RMS=%.3f V | Freq w=%.3f", signalRMS, measuredFrequency);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.05f, 0.05f, 0.07f, 1.0f));
    ImGui::PlotLines("##Oscilloscope", plotCache.data(), 120, 0, nullptr, -1.5f, 1.5f, ImVec2(-1, 50));
    ImGui::PopStyleColor();

    // Call split exporter code seamlessly inside our primary window layout lifetime 
    renderEXRExportLayout(engine); 

    ImGui::End();
}

