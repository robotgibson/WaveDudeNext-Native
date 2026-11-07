#include "zoom_pip_ui.h"
#include "imgui.h"
#include "UIRegistry.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>
#include <cstdio>

void renderAnalyticsFooterPanel(WaveEngine& engine, const std::vector<uint32_t>& pixelBuffer, int& pX, int& pY) {
    ImGui::SetNextWindowPos(ImVec2(320, 500), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(810, 210), ImGuiCond_Always); 
    
    if (!ImGui::Begin("Analysis Deck Footer Workspace", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar)) {
        ImGui::End();
        return;
    }

    // -------------------------------------------------------------------------
    // SUB-PANEL A: FATBITS ZOOM LENS (LEFT COLUMN)
    // -------------------------------------------------------------------------
    ImGui::BeginChild("##SubPanelFatBits", ImVec2(140, 180), false, ImGuiWindowFlags_NoScrollbar);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 groupStartPos = ImGui::GetCursorScreenPos();
    
    int w = engine.getGridWidth();
    int h = engine.getGridHeight();
    const int radius = 6; 
    const float blockSize = 10.0f; 

    if (!pixelBuffer.empty() && w > 0 && h > 0) {
        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                int sampleX = std::clamp(pX + dx, 0, w - 1);
                int sampleY = std::clamp(pY + dy, 0, h - 1);
                uint32_t pixel = pixelBuffer[sampleY * w + sampleX];
                
                uint8_t b = (pixel >> 0) & 0xFF;
                uint8_t g = (pixel >> 8) & 0xFF;
                uint8_t r = (pixel >> 16) & 0xFF;
                
                ImVec2 bMin = ImVec2(groupStartPos.x + (dx + radius) * blockSize, groupStartPos.y + (dy + radius) * blockSize);
                ImVec2 bMax = ImVec2(bMin.x + blockSize, bMin.y + blockSize);
                drawList->AddRectFilled(bMin, bMax, IM_COL32(r, g, b, 255));
            }
        }
        ImVec2 cMin = ImVec2(groupStartPos.x + radius * blockSize, groupStartPos.y + radius * blockSize);
        ImVec2 cMax = ImVec2(cMin.x + blockSize, cMin.y + blockSize);
        drawList->AddRect(cMin, cMax, IM_COL32(0, 255, 255, 240), 0.0f, 0, 1.5f);
    }
    ImGui::EndChild();

    ImGui::SameLine(155);

    // -------------------------------------------------------------------------
    // SUB-PANEL B: SLIDER CONTROLS (CENTER COLUMN)
    // -------------------------------------------------------------------------
    float peakLimitThreshold = engine.getTargetAmpLimit(); // Slider treats limit explicitly as V_peak
    float targetFreqThreshold = engine.getTargetFreqThreshold();
    static float frequencyTolerance = 0.008f;  
    
    static std::vector<float> vPlotCache(120, 0.0f);
    static std::vector<float> fPlotCache(120, 0.0f);

    float currentSample = engine.sampleAmplitude(pX, pY);

    float sumSquares = 0.0f;
    for (float val : vPlotCache) { sumSquares += val * val; }
    float signalRMS = std::sqrt(sumSquares / (vPlotCache.empty() ? 1.0f : vPlotCache.size()));

    int zeroCrossings = 0;
    for (size_t i = 1; i < vPlotCache.size(); ++i) {
        if ((vPlotCache[i - 1] >= 0.0f && vPlotCache[i] < 0.0f) || (vPlotCache[i - 1] < 0.0f && vPlotCache[i] >= 0.0f)) zeroCrossings++;
    }
    float measuredFrequency = (signalRMS > 0.01f) ? (zeroCrossings * 3.14159265f) / (vPlotCache.size() * 2.0f) : 0.0f;

    if (!engine.getPauseState()) {
        vPlotCache.erase(vPlotCache.begin());
        vPlotCache.push_back(currentSample);

        fPlotCache.erase(fPlotCache.begin());
        fPlotCache.push_back(measuredFrequency);
    }

    // Capture precise peak metrics out of the cycle log window
    float vMin = vPlotCache.empty() ? 0.0f : *std::min_element(vPlotCache.begin(), vPlotCache.end());
    float vMax = vPlotCache.empty() ? 0.0f : *std::max_element(vPlotCache.begin(), vPlotCache.end());
    float signalPeakMax = std::max(std::abs(vMin), std::abs(vMax));
    float signalVpp = vMax - vMin;

    ImGui::BeginChild("##SubPanelSliders", ImVec2(270, 180), false, ImGuiWindowFlags_NoScrollbar);
    ImGui::Text("Probe Node Placement:");
    ImGui::SetNextItemWidth(140); PrecisionSlider<int>("Coord X", &pX, 0, w > 0 ? w : 800, 1, "pX=%d");
    ImGui::SetNextItemWidth(140); PrecisionSlider<int>("Coord Y", &pY, 0, h > 0 ? h : 600, 1, "pY=%d");
    
    ImGui::Separator();
    ImGui::Text("Tuning Adjusters:");
    
    if (PrecisionSlider<float>("Amp Lim", &peakLimitThreshold, 0.001f, 1.20f, 0.001f, "V=%.3f")) {
        engine.setTargetAmpLimit(peakLimitThreshold);
    }
    ImGui::SetNextItemWidth(140); 
    if (PrecisionSlider<float>("Freq Lim", &targetFreqThreshold, 0.03f, 0.18f, 0.001f, "w=%.3f")) {
        engine.setTargetFreqThreshold(targetFreqThreshold);
    }
    ImGui::EndChild();

    ImGui::SameLine(435);

    // -------------------------------------------------------------------------
    // SUB-PANEL C: PARALLEL RMS vs VPP LOGIC INDICATORS
    // -------------------------------------------------------------------------
    ImGui::BeginChild("##SubPanelLogicOutputs", ImVec2(210, 180), false, ImGuiWindowFlags_NoScrollbar);
    
    // FIXED: Adjusted formulas so RMS logic maps onto standard harmonic threshold scaling (V_peak / sqrt(2))
    float expectedRMSThreshold = peakLimitThreshold * 0.707106f;
    bool rmsLogicHigh = (signalRMS >= expectedRMSThreshold);
    
    // FIXED: Adjusted Vpp logic to test the true peak bounds envelope correctly (Max Peak >= V_peak limit)
    bool vppLogicHigh = (signalPeakMax >= peakLimitThreshold); 
    
    bool fLogicHigh = (signalRMS > 0.01f) && 
                      ((std::abs(measuredFrequency - targetFreqThreshold) <= frequencyTolerance) || 
                       (std::abs(measuredFrequency - (targetFreqThreshold * 0.5f)) <= frequencyTolerance));

    ImGui::Text("Gating Truth Results:");
    ImGui::Spacing();
    
    ImGui::Text("LOGIC (RMS):"); ImGui::SameLine();
    if (!rmsLogicHigh) ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "F");
    else               ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.3f, 1.0f), "T");

    ImGui::Text("LOGIC (Vpp):"); ImGui::SameLine();
    if (!vppLogicHigh) ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "F");
    else               ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.3f, 1.0f), "T");

    ImGui::Text("LOGIC (Freq):"); ImGui::SameLine();
    if (!fLogicHigh)   ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "F");
    else               ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "T");

    ImGui::Separator();
    ImGui::TextDisabled("RMS:%.3fV | Vpp:%.3fV", signalRMS, signalVpp);
    ImGui::TextDisabled("Measured Freq w: %.3f", measuredFrequency);
    ImGui::EndChild();

    ImGui::SameLine(655);

    // -------------------------------------------------------------------------
    // SUB-PANEL D: OSCILLOSCOPES WITH DYNAMIC MIN/MAX OVERLAYS
    // -------------------------------------------------------------------------
    ImGui::BeginChild("##SubPanelOscilloscopes", ImVec2(145, 180), false, ImGuiWindowFlags_NoScrollbar);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.04f, 0.04f, 0.06f, 1.0f));
    
    float fMin = fPlotCache.empty() ? 0.0f : *std::min_element(fPlotCache.begin(), fPlotCache.end());
    float fMax = fPlotCache.empty() ? 0.0f : *std::max_element(fPlotCache.begin(), fPlotCache.end());

    // --- 1. Voltage Waveform Plot ---
    ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.0f, 0.9f, 1.0f, 1.0f));
    ImGui::PlotLines("##VOscilloscope", vPlotCache.data(), (int)vPlotCache.size(), 0, nullptr, -1.5f, 1.5f, ImVec2(-1, 45));
    ImGui::PopStyleColor();
    
    ImVec2 vTextPos = ImGui::GetItemRectMin();
    vTextPos.x += 4.0f; vTextPos.y += 2.0f;
    char vStr[32]; std::snprintf(vStr, sizeof(vStr), "V:%+.2f", currentSample);
    ImGui::GetWindowDrawList()->AddText(vTextPos, IM_COL32(255, 255, 255, 220), vStr);
    
    ImGui::TextDisabled("Mn:%+.2f Mx:%+.2f", vMin, vMax);
    ImGui::Spacing();

    // --- 2. Frequency Spectral Shift Plot ---
    ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(1.0f, 0.6f, 0.0f, 1.0f));
    ImGui::PlotLines("##FOscilloscope", fPlotCache.data(), (int)fPlotCache.size(), 0, nullptr, 0.00f, 0.20f, ImVec2(-1, 45));
    ImGui::PopStyleColor();
    
    ImVec2 fTextPos = ImGui::GetItemRectMin();
    fTextPos.x += 4.0f; fTextPos.y += 2.0f;
    char fStr[32]; std::snprintf(fStr, sizeof(fStr), "w:%.3f", measuredFrequency);
    ImGui::GetWindowDrawList()->AddText(fTextPos, IM_COL32(255, 255, 255, 220), fStr);
    
    ImGui::TextDisabled("Mn:%.3f Mx:%.3f", fMin, fMax);
    
    ImGui::PopStyleColor(); 
    ImGui::EndChild();

    ImGui::End();
}
