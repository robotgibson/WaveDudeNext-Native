#include "exr_exporter_ui.h"
#include "stencil_selector_ui.h"
#include "imgui.h"
#include "engine/WaveEngine.hpp"
#include "UIRegistry.hpp"
#include <vector>
#include <cstring>
#include <cmath>
#include <numeric>
#include <fstream>
#include <sstream>
#include <iostream>

void renderPrismSettings(WaveEngine& engine);
void renderEmitterMatrix(WaveEngine& engine, float& sliderAmp, float& sliderFreq, bool& bitA, bool& bitB, int& pX, int& pY);

// --- Lightweight Inline Static Configuration Profile Parser ---
static void syncUIPropertiesFromJSON(WaveEngine& engine, int targetSceneID, int& pX, int& pY) {
    std::string configPath = "config/scene_profiles.json";
    std::ifstream file(configPath);
    if (!file.is_open()) {
        file.open("../" + configPath); // Fallback lookup context for build/ tracking folders
    }
    if (!file.is_open()) return;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string contents = buffer.str();

    size_t scenePos = 0;
    while ((scenePos = contents.find("\"id\":", scenePos)) != std::string::npos) {
        size_t valStart = scenePos + 5;
        size_t valEnd = contents.find_first_of(",\n}", valStart);
        int currentID = std::stoi(contents.substr(valStart, valEnd - valStart));

        if (currentID == targetSceneID) {
            auto getFloatVal = [&](const std::string& attr) {
                size_t attrPos = contents.find("\"" + attr + "\":", scenePos);
                if (attrPos != std::string::npos) {
                    size_t s = attrPos + attr.length() + 3;
                    size_t e = contents.find_first_of(",\n}", s);
                    return std::stof(contents.substr(s, e - s));
                }
                return -1.0f;
            };

            float limit = getFloatVal("amp_limit");
            float jsonX = getFloatVal("probe_x");
            float jsonY = getFloatVal("probe_y");
            float jsonW = getFloatVal("target_w");

            if (limit > 0.0f) engine.setTargetAmpLimit(limit);
            if (jsonW > 0.0f) engine.setTargetFreqThreshold(jsonW);
            if (jsonX > 0.0f) pX = static_cast<int>(jsonX);
            if (jsonY > 0.0f) pY = static_cast<int>(jsonY);
            return;
        }
        scenePos++;
    }
}

void renderTuningDesk(WaveEngine& engine, char* pathBuffer, size_t bufferSize, 
                      float& sliderAmp, float& sliderFreq, float& sliderPhase,
                      bool& bitA, bool& bitB, int& paletteIdx, int& pX, int& pY) {
    
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(300, 700), ImGuiCond_Always);
    ImGui::Begin("Global Configuration Suite", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    
    // =========================================================================
    // SECTION 1: SIMULATION SCENE PRESETS (DYNAMIC SELECTION WITH DUAL DOCKING UPDATES)
    // =========================================================================
    if (ImGui::CollapsingHeader("Simulation Scene Profiles", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("AND Gate Circuit", ImVec2(134, 24))) {
            engine.switchToAndGate(); sliderFreq = 0.08f; engine.setGlobalFrequency(0.08f); engine.setSelectedNodeIndex(-1);
            syncUIPropertiesFromJSON(engine, 0, pX, pY);
        }
        ImGui::SameLine();
        if (ImGui::Button("OR Gate Circuit", ImVec2(134, 24))) {
            engine.switchToOrGate(); sliderFreq = 0.08f; engine.setGlobalFrequency(0.08f); engine.setSelectedNodeIndex(-1);
            syncUIPropertiesFromJSON(engine, 3, pX, pY);
        }
        if (ImGui::Button("NOT Gate Circuit", ImVec2(134, 24))) {
            engine.switchToNotGate(); sliderFreq = 0.08f; engine.setGlobalFrequency(0.08f); engine.setSelectedNodeIndex(-1);
            syncUIPropertiesFromJSON(engine, 4, pX, pY);
        }
        ImGui::SameLine();
        if (ImGui::Button("XOR Gate Circuit", ImVec2(134, 24))) {
            engine.switchToXorGate(); sliderFreq = 0.08f; engine.setGlobalFrequency(0.08f); engine.setSelectedNodeIndex(-1);
            syncUIPropertiesFromJSON(engine, 5, pX, pY);
        }

        if (ImGui::Button("Glass Prism Layout", ImVec2(-1, 26))) {
            engine.switchToPrismScene(); sliderFreq = 0.07f; engine.setGlobalFrequency(0.07f); pX = 550; pY = 380; 
            engine.setSelectedNodeIndex(-1);
            engine.setTargetAmpLimit(0.055f);
            engine.setTargetFreqThreshold(0.080f);
        }
    }

    // =========================================================================
    // SECTION 2: FILE OPERATIONS & CUSTOM MAPS
    // =========================================================================
    if (ImGui::CollapsingHeader("File Operations & Asset Canvas", ImGuiTreeNodeFlags_None)) {
        if (ImGui::Button("Save Image Layout (PNG Snapshot)", ImVec2(-1, 28))) {
            engine.triggerSnapshotCapture();
        }
        
        ImGui::Separator();
        ImGui::Text("Load Custom Map Image (.png):");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##PathBox", pathBuffer, bufferSize);
        if (ImGui::Button("Compile Custom Scene Canvas Map", ImVec2(-1, 28))) {
            if (engine.loadSceneFromImage(pathBuffer)) {
                pX = 400; pY = 300; engine.setSelectedNodeIndex(-1);
            }
        }
    }

    // =========================================================================
    // SECTION 3: PROCEDURAL LAB ASSETS TOOLKIT
    // =========================================================================
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

    renderStencilPrecisionControls(engine);
    renderEXRExportLayout(engine); 

    ImGui::End();
}

