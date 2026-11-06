#include "imgui.h"
#include "engine/WaveEngine.hpp"
#include "UIRegistry.hpp" 
#include <string>
#include <vector>

void renderEmitterMatrix(WaveEngine& engine, float& sliderAmp, float& sliderFreq, bool& bitA, bool& bitB, int& pX, int& pY) {
    auto& nodesList = engine.getCustomSourceNodes();
    const char* palettes[] = { "Classic Field Tone", "Green / Magenta Field", "Monochrome Radar Glow" };
    static int paletteIdx = 0;

    ImGui::Separator();
    if (!engine.getPauseState()) {
        if (ImGui::Button("Pause Execution Engine", ImVec2(-1, 30))) { engine.setPauseState(true); }
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
        if (ImGui::Button("Resume Physics Loop", ImVec2(-1, 30))) { engine.setPauseState(false); }
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
    ImGui::Text("Visualization Output Layer:");
    bool mode = engine.getIntensityMode();
    if (ImGui::RadioButton("Amplitude Fields (Phase Ripples)", !mode)) { engine.setIntensityMode(false); }
    if (ImGui::RadioButton("Intensity Focus (Steady Beams)", mode))    { engine.setIntensityMode(true); }

    ImGui::Spacing();
    ImGui::Text("Render Color Scheme:");
    ImGui::SetNextItemWidth(-1);
    if (ImGui::Combo("##PaletteCombo", &paletteIdx, palettes, 3)) { engine.setColorPalette(paletteIdx); }

    ImGui::Separator();
    ImGui::Text("Independent Emitter Channels Matrix:");
    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "Total Live Sources: %zu / 16 Max", nodesList.size());
    if (ImGui::Button("Clear All Emitters", ImVec2(-1, 24))) { engine.clearIndependentSourceNodes(); }
    
    if (!nodesList.empty()) {
        ImGui::BeginChild("##MatrixScrollContainer", ImVec2(-1, 100), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
        for (size_t i = 0; i < nodesList.size(); ++i) {
            std::string labelText = "Ch #" + std::to_string(i + 1);
            if (engine.getActiveSceneMode() == 0) {
                if (i == 0 || i == 2) labelText += " [Input A]";
                else if (i == 1 || i == 4) labelText += " [Input B]";
                else if (i == 3) labelText += " [Cancel Phase]";
            } else {
                int absX = engine.getGlobalOriginX() + nodesList[i].offsetX;
                int absY = engine.getGlobalOriginY() + nodesList[i].offsetY;
                labelText += " (Abs X:" + std::to_string(absX) + ", Y:" + std::to_string(absY) + ")";
            }
            if (ImGui::Checkbox(labelText.c_str(), &nodesList[i].isActive)) {
                if (engine.getActiveSceneMode() == 0) {
                    if (i == 0 || i == 2) bitA = nodesList[i].isActive;
                    if (i == 1 || i == 4) bitB = nodesList[i].isActive;
                    engine.toggleInputs(bitA, bitB);
                }
            }
        }
        ImGui::EndChild();
        
        ImGui::Spacing();
        ImGui::Text("Global Cluster Position Center:");
        int originX = engine.getGlobalOriginX(); int originY = engine.getGlobalOriginY();
        if (PrecisionSlider<int>("Origin X", &originX, -300, 800, 1) || PrecisionSlider<int>("Origin Y", &originY, -300, 600, 1)) {
            engine.setGlobalOrigin(originX, originY);
        }

        ImGui::Spacing();
        ImGui::Text("Tune Focused Channel Relative Offsets:");
        int focusIdx = engine.getSelectedNodeIndex();
        std::string comboLabel = (focusIdx == -1) ? "Select..." : "Edit Laser #" + std::to_string(focusIdx + 1);
        
        ImGui::SetNextItemWidth(-1);
        if (ImGui::BeginCombo("##SelectNodeCombo", comboLabel.c_str())) {
            if (ImGui::Selectable("Select...", focusIdx == -1)) { focusIdx = -1; engine.setSelectedNodeIndex(-1); }
            for (size_t i = 0; i < nodesList.size(); ++i) {
                std::string itemTitle = "Laser Pin #" + std::to_string(i + 1) + " (Offset X:" + std::to_string(nodesList[i].offsetX) + ")";
                if (ImGui::Selectable(itemTitle.c_str(), focusIdx == static_cast<int>(i))) { focusIdx = static_cast<int>(i); engine.setSelectedNodeIndex(focusIdx); }
            }
            ImGui::EndCombo();
        }
        
        if (focusIdx >= 0 && focusIdx < static_cast<int>(nodesList.size())) {
            auto& activeLaser = nodesList[focusIdx];
            ImGui::Spacing();
            PrecisionSlider<int>("Offset X", &activeLaser.offsetX, -400, 400, 1, "dX = %d");
            PrecisionSlider<int>("Offset Y", &activeLaser.offsetY, -400, 400, 1, "dY = %d");
            PrecisionSlider<float>("Gain",     &activeLaser.amplitudeScale, 0.0f, 3.0f, 0.05f, "Gain=%.2f");
            // FIXED: Added contextual ##Channel suffix descriptor block to distinguish this from global controls
            PrecisionSlider<float>("Freq##Channel", &activeLaser.frequencyOverride, 0.03f, 0.18f, 0.001f, "w=%.3f");
            PrecisionSlider<float>("Phase",    &activeLaser.phaseOffsetDeg, -180.0f, 180.0f, 1.0f, "%.1f Deg");
        }
    }

    ImGui::Separator();
    ImGui::Text("Global Masters Scaling:");
    PrecisionSlider<float>("Power", &sliderAmp, 0.0f, 2.0f, 0.05f, "A=%.2f");
    // FIXED: Added contextual ##Global suffix descriptor block to eliminate internal hashing overlap collisions
    PrecisionSlider<float>("Freq##Global", &sliderFreq, 0.04f, 0.16f, 0.002f, "w=%.3f");
    engine.setGlobalAmplitude(sliderAmp);
    engine.setGlobalFrequency(sliderFreq);
}

