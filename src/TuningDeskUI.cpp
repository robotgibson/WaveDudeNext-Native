#include "imgui.h"
#include "engine/WaveEngine.hpp"
#include <vector>
#include <cstring>
#include <string>

void renderTuningDesk(WaveEngine& engine, char* pathBuffer, size_t bufferSize, 
                      float& sliderAmp, float& sliderFreq, float& sliderPhase,
                      bool& bitA, bool& bitB, int& paletteIdx, int& pX, int& pY) {
    
    const char* palettes[] = { "Classic Field Tone", "Green / Magenta Field", "Monochrome Radar Glow" };
    auto& nodesList = engine.getCustomSourceNodes();

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(300, 700), ImGuiCond_Always);
    ImGui::Begin("Wave Tuning Desk", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    
    ImGui::Text("Simulation Scene Profile:");
    if (ImGui::Button("Gates", ImVec2(134, 30))) {
        engine.resetToProceduralGates();
        sliderFreq = 0.08f; engine.setGlobalFrequency(0.08f);
        pX = 360; pY = 185; 
    }
    ImGui::SameLine();
    if (ImGui::Button("Glass Prism", ImVec2(134, 30))) {
        engine.switchToPrismScene();
        sliderFreq = 0.07f; engine.setGlobalFrequency(0.07f); 
        pX = 550; pY = 380; 
    }

    ImGui::Spacing();
    if (ImGui::CollapsingHeader("Procedural Lab Asset Toolkit", ImGuiTreeNodeFlags_None)) {
        if (ImGui::Button("Load Double Slit Layout", ImVec2(-1, 26))) {
            engine.generateNativeDoubleSlitPNG(); std::strncpy(pathBuffer, "double_slit_experiment.bmp", bufferSize);
            if (engine.loadSceneFromImage(pathBuffer)) { pX = 320; pY = 300; }
        }
        if (ImGui::Button("Load Spherical Ball Lens", ImVec2(-1, 26))) {
            engine.generateNativeBallLensPNG(); std::strncpy(pathBuffer, "focusing_ball_lens.bmp", bufferSize);
            if (engine.loadSceneFromImage(pathBuffer)) { pX = 580; pY = 300; }
        }
        if (ImGui::Button("Load Fiber Network Splitter", ImVec2(-1, 26))) {
            engine.generateNativeFiberNetworkPNG(); std::strncpy(pathBuffer, "fiber_splitter_network.bmp", bufferSize);
            if (engine.loadSceneFromImage(pathBuffer)) { pX = 450; pY = 300; }
        }
    }

    if (engine.getActiveSceneMode() == 2) {
        PrismPart* prism = static_cast<PrismPart*>(engine.getPrismPart());
        if (prism) {
            ImGui::Spacing();
            if (ImGui::CollapsingHeader("Prism Geometric Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                int posX = prism->getX(); int posY = prism->getY(); int size = prism->getSize(); float rot = prism->getRotation();
                bool changed = false;
                if (ImGui::SliderInt("Prism X Pos", &posX, 50, 750))    { prism->setLocation(posX, posY); changed = true; }
                if (ImGui::SliderInt("Prism Y Pos", &posY, 50, 550))    { prism->setLocation(posX, posY); changed = true; }
                if (ImGui::SliderInt("Prism Radius", &size, 40, 300)) { prism->setSize(size); changed = true; }
                if (ImGui::SliderFloat("Rotation Angle", &rot, -180.0f, 180.0f, "%.1f Deg")) { prism->setRotation(rot); changed = true; }
                if (changed) { engine.initSimulationSpace(); }
            }
        }
    }

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
        // Vertical Active State Selection Checklist Box Row Container (Wiped out legacy global A/B blocks)
        ImGui::BeginChild("##MatrixScrollContainer", ImVec2(-1, 110), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
        for (size_t i = 0; i < nodesList.size(); ++i) {
            std::string labelText = "Ch #" + std::to_string(i + 1);
            if (engine.getActiveSceneMode() == 0) {
                if (i == 0 || i == 2) labelText += " [Input A Port]";
                else if (i == 1 || i == 4) labelText += " [Input B Port]";
                else if (i == 3) labelText += " [Cancel Phase Ref]";
            } else {
                labelText += " (X:" + std::to_string(nodesList[i].x) + ", Y:" + std::to_string(nodesList[i].y) + ")";
            }
            
            // Pass checkbox changes dynamically directly down to sync input signals
            if (ImGui::Checkbox(labelText.c_str(), &nodesList[i].isActive)) {
                if (engine.getActiveSceneMode() == 0) {
                    // Sync internal legacy bit multipliers if evaluating gates scene profiles
                    if (i == 0 || i == 2) bitA = nodesList[0].isActive;
                    if (i == 1 || i == 4) bitB = nodesList[1].isActive;
                    engine.toggleInputs(bitA, bitB);
                }
            }
        }
        ImGui::EndChild();
        
        ImGui::Spacing();
        ImGui::Text("Tune Focused Channel Attributes:");
        
        // FIXED: Restored dedicated select dropdown with neutral 'Select...' clear state
        int focusIdx = engine.getSelectedNodeIndex();
        std::string comboLabel = (focusIdx == -1) ? "Select..." : "Edit Laser #" + std::to_string(focusIdx + 1);
        
        ImGui::SetNextItemWidth(-1);
        if (ImGui::BeginCombo("##SelectNodeCombo", comboLabel.c_str())) {
            if (ImGui::Selectable("Select...", focusIdx == -1)) {
                focusIdx = -1; engine.setSelectedNodeIndex(-1);
            }
            for (size_t i = 0; i < nodesList.size(); ++i) {
                std::string itemTitle = "Laser Pin #" + std::to_string(i + 1) + " (X:" + std::to_string(nodesList[i].x) + ")";
                if (ImGui::Selectable(itemTitle.c_str(), focusIdx == static_cast<int>(i))) {
                    focusIdx = static_cast<int>(i); engine.setSelectedNodeIndex(focusIdx);
                }
            }
            ImGui::EndCombo();
        }
        
        // Render node-specific sliders only if a valid laser index is selected
        if (focusIdx >= 0 && focusIdx < static_cast<int>(nodesList.size())) {
            auto& activeLaser = nodesList[focusIdx];
            ImGui::Spacing();
            ImGui::SliderInt("Channel Coord X", &activeLaser.x, 10, 790);
            ImGui::SliderInt("Channel Coord Y", &activeLaser.y, 10, 590);
            ImGui::SliderFloat("Channel Power", &activeLaser.amplitudeScale, 0.0f, 3.0f, "Gain=%.2f");
            ImGui::SliderFloat("Channel Freq", &activeLaser.frequencyOverride, 0.03f, 0.18f, "w=%.3f");
            ImGui::SliderFloat("Channel Phase", &activeLaser.phaseOffsetDeg, -180.0f, 180.0f, "%.1f Deg");
        }
    }

    ImGui::Separator();
    ImGui::Text("Global Masters Scaling:");
    if (ImGui::SliderFloat("Master Power", &sliderAmp, 0.0f, 2.0f, "A=%.2f")) { engine.setGlobalAmplitude(sliderAmp); }
    if (ImGui::SliderFloat("Master Freq", &sliderFreq, 0.04f, 0.16f, "w=%.3f")) { engine.setGlobalFrequency(sliderFreq); }

    ImGui::Separator();
    ImGui::Text("Probe Node Trace (X:%d, Y:%d):", pX, pY);
    
    static std::vector<float> plotCache(120, 0.0f);
    if (!engine.getPauseState()) {
        plotCache.erase(plotCache.begin()); plotCache.push_back(engine.sampleAmplitude(pX, pY));
    }
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.05f, 0.05f, 0.07f, 1.0f));
    ImGui::PlotLines("##Oscilloscope", plotCache.data(), 120, 0, nullptr, -1.5f, 1.5f, ImVec2(-1, 80));
    ImGui::PopStyleColor();

    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 35);
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "Performance: %.1f FPS (%.2f ms)", 
        ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

    ImGui::End();
}
