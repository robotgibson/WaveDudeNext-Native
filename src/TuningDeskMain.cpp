#include "imgui.h"
#include "engine/WaveEngine.hpp"
#include <vector>
#include <cstring>

// Forward declare split UI panels cleanly across compilation units
void renderPrismSettings(WaveEngine& engine);
void renderEmitterMatrix(WaveEngine& engine, float& sliderAmp, float& sliderFreq, bool& bitA, bool& bitB, int& pX, int& pY);

void renderTuningDesk(WaveEngine& engine, char* pathBuffer, size_t bufferSize, 
                      float& sliderAmp, float& sliderFreq, float& sliderPhase,
                      bool& bitA, bool& bitB, int& paletteIdx, int& pX, int& pY) {
    
    const char* palettes[] = { "Classic Field Tone", "Green / Magenta Field", "Monochrome Radar Glow" };

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(300, 700), ImGuiCond_Always);
    ImGui::Begin("Wave Tuning Desk", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    
    ImGui::Text("Simulation Scene Profile:");
    if (ImGui::Button("Gates", ImVec2(134, 30))) {
        engine.resetToProceduralGates(); sliderFreq = 0.08f; engine.setGlobalFrequency(0.08f); pX = 360; pY = 185; 
    }
    ImGui::SameLine();
    if (ImGui::Button("Glass Prism", ImVec2(134, 30))) {
        engine.switchToPrismScene(); sliderFreq = 0.07f; engine.setGlobalFrequency(0.07f); pX = 550; pY = 380; 
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

    // Call split sub-panels natively based on current engine state rules
    renderPrismSettings(engine);
    renderEmitterMatrix(engine, sliderAmp, sliderFreq, bitA, bitB, pX, pY);

    ImGui::Separator();
    ImGui::Text("Probe Node Trace (X:%d, Y:%d):", pX, pY);
    static std::vector<float> plotCache(120, 0.0f);
    if (!engine.getPauseState()) {
        plotCache.erase(plotCache.begin()); plotCache.push_back(engine.sampleAmplitude(pX, pY));
    }
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.05f, 0.05f, 0.07f, 1.0f));
    ImGui::PlotLines("##Oscilloscope", plotCache.data(), 120, 0, nullptr, -1.5f, 1.5f, ImVec2(-1, 80));
    ImGui::PopStyleColor();

    ImGui::End();
}
