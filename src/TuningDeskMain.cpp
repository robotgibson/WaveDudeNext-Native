#include "imgui.h"
#include "engine/WaveEngine.hpp"
#include <vector>
#include <cstring>
#include <cmath>
#include <numeric>

void renderPrismSettings(WaveEngine& engine);
void renderEmitterMatrix(WaveEngine& engine, float& sliderAmp, float& sliderFreq, bool& bitA, bool& bitB, int& pX, int& pY);

// Link access back to your shared precision arrow slider template mapping natively
template<typename T>
extern bool PrecisionSlider(const char* label, T* value, T minVal, T maxVal, T step, const char* format = nullptr);

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

    float currentSample = engine.sampleAmplitude(pX, pY);
    if (!engine.getPauseState()) {
        plotCache.erase(plotCache.begin());
        plotCache.push_back(currentSample);
    }

    float sumSquares = 0.0f;
    for (float val : plotCache) { sumSquares += val * val; }
    float signalRMS = std::sqrt(sumSquares / plotCache.size());

    // FIXED: Upgraded threshold limit input to use PrecisionSlider tracking with 0.01 step jumps
    PrecisionSlider<float>("Threshold", &comparisonThreshold, 0.01f, 1.20f, 0.01f, "Limit = %.2f");

    bool isGateTriggered = (signalRMS >= comparisonThreshold);
    if (!isGateTriggered) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.3f, 1.0f));
        ImGui::Text("LOGIC LEVEL OUTPUT: 0 (LOW)");
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.0f, 1.0f));
        ImGui::Text("LOGIC LEVEL OUTPUT: 1 (HIGH)");
        ImGui::PopStyleColor();
    }
    ImGui::TextDisabled("Signal Root-Mean-Square: %.3f V", signalRMS);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.05f, 0.05f, 0.07f, 1.0f));
    ImGui::PlotLines("##Oscilloscope", plotCache.data(), 120, 0, nullptr, -1.5f, 1.5f, ImVec2(-1, 65));
    ImGui::PopStyleColor();

    ImGui::End();
}
