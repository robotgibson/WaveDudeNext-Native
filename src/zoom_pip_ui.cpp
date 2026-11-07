#include "zoom_pip_ui.h"
#include "imgui.h"
#include "UIRegistry.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>
#include <cstdio>

void calculateWaveMetrics(WaveEngine& engine, int pX, int pY, const std::vector<float>& vPlotCache,
                          float& signalRMS, float& measuredFrequency, float& signalPeakMax, float& signalVpp);
void updateSettleTimers(WaveEngine& engine, bool rmsHigh, bool vppHigh, bool freqHigh, bool globalHigh,
                        float& rmsMS, float& vppMS, float& freqMS, float& globalMS);

void renderAnalyticsFooterPanel(WaveEngine& engine, const std::vector<uint32_t>& pixelBuffer, int& pX, int& pY) {
    ImGui::SetNextWindowPos(ImVec2(260, 500), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(870, 210), ImGuiCond_Always); 
    
    if (!ImGui::Begin("Analysis Deck Footer Workspace", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar)) {
        ImGui::End();
        return;
    }

    // --- SUB-PANEL A: FATBITS ZOOM VIEW ---
    ImGui::BeginChild("##SubPanelFatBits", ImVec2(140, 180), false, ImGuiWindowFlags_NoScrollbar);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 groupStartPos = ImGui::GetCursorScreenPos();
    int w = engine.getGridWidth(), h = engine.getGridHeight();
    const int radius = 6; const float blockSize = 10.0f;

    if (!pixelBuffer.empty() && w > 0 && h > 0) {
        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                int sX = std::clamp(pX + dx, 0, w - 1), sY = std::clamp(pY + dy, 0, h - 1);
                uint32_t pixel = pixelBuffer[sY * w + sX];
                drawList->AddRectFilled(ImVec2(groupStartPos.x + (dx + radius) * blockSize, groupStartPos.y + (dy + radius) * blockSize),
                                       ImVec2(groupStartPos.x + (dx + radius + 1) * blockSize, groupStartPos.y + (dy + radius + 1) * blockSize),
                                       IM_COL32((pixel >> 16) & 0xFF, (pixel >> 8) & 0xFF, pixel & 0xFF, 255));
            }
        }
        drawList->AddRect(ImVec2(groupStartPos.x + radius * blockSize, groupStartPos.y + radius * blockSize),
                          ImVec2(groupStartPos.x + (radius + 1) * blockSize, groupStartPos.y + (radius + 1) * blockSize), IM_COL32(0, 255, 255, 240), 0.0f, 0, 1.5f);
    }
    ImGui::EndChild();
    ImGui::SameLine(155);

    // --- DATA LOGIC INFRASTRUCTURE CACHE UPDATES ---
    static std::vector<float> vPlotCache(120, 0.0f), fPlotCache(120, 0.0f);
    static float lastRMS = 0.50f, lastVpp = 1.40f, lastFreq = 0.039f;

    float currentSample = engine.sampleAmplitude(pX, pY);
    float signalRMS = 0.0f, measuredFrequency = 0.0f, signalPeakMax = 0.0f, signalVpp = 0.0f;
    
    calculateWaveMetrics(engine, pX, pY, vPlotCache, signalRMS, measuredFrequency, signalPeakMax, signalVpp);

    if (!engine.getPauseState()) {
        vPlotCache.erase(vPlotCache.begin()); vPlotCache.push_back(currentSample);
        fPlotCache.erase(fPlotCache.begin()); fPlotCache.push_back(measuredFrequency);
    }

    float vMin = vPlotCache.empty() ? 0.0f : *std::min_element(vPlotCache.begin(), vPlotCache.end());
    float vMax = vPlotCache.empty() ? 0.0f : *std::max_element(vPlotCache.begin(), vPlotCache.end());
    float fMin = fPlotCache.empty() ? 0.0f : *std::min_element(fPlotCache.begin(), fPlotCache.end());
    float fMax = fPlotCache.empty() ? 0.0f : *std::max_element(fPlotCache.begin(), fPlotCache.end());
    
    signalPeakMax = std::max(std::abs(vMin), std::abs(vMax));
    signalVpp = vMax - vMin;

    // --- SUB-PANEL B: SLIDERS CONFIG PANEL ---
    float peakLimitThreshold = engine.getTargetAmpLimit(), targetFreqThreshold = engine.getTargetFreqThreshold();
    ImGui::BeginChild("##SubPanelSliders", ImVec2(245, 180), false, ImGuiWindowFlags_NoScrollbar);
    ImGui::Text("Probe Node Placement:");
    ImGui::SetNextItemWidth(125); PrecisionSlider<int>("Coord X", &pX, 0, w > 0 ? w : 800, 1, "pX=%d");
    ImGui::SetNextItemWidth(125); PrecisionSlider<int>("Coord Y", &pY, 0, h > 0 ? h : 600, 1, "pY=%d");
    ImGui::Separator(); ImGui::Text("Tuning Adjusters:");
    ImGui::SetNextItemWidth(125); if (PrecisionSlider<float>("Amp Lim", &peakLimitThreshold, 0.001f, 1.20f, 0.001f, "Vp=%.3f")) engine.setTargetAmpLimit(peakLimitThreshold);
    ImGui::SetNextItemWidth(125); if (PrecisionSlider<float>("Freq Lim", &targetFreqThreshold, 0.03f, 0.18f, 0.001f, "w=%.3f")) engine.setTargetFreqThreshold(targetFreqThreshold);
    ImGui::EndChild();
    ImGui::SameLine(410);

    // --- SUB-PANEL C: TABULAR TRUTH MATRIX ROW DECK ---
    ImGui::BeginChild("##SubPanelLogicOutputs", ImVec2(300, 180), false, ImGuiWindowFlags_NoScrollbar);
    float expectedRMSThreshold = peakLimitThreshold * 0.707106f, expectedVppThreshold = peakLimitThreshold * 2.0f;
    bool rHigh = (signalRMS >= expectedRMSThreshold), vHigh = (signalPeakMax >= peakLimitThreshold);
    bool fHigh = (signalRMS > 0.01f) && ((std::abs(measuredFrequency - targetFreqThreshold) <= 0.008f) || (std::abs(measuredFrequency - (targetFreqThreshold * 0.5f)) <= 0.008f));
    
    bool hasConsensus = (rHigh == vHigh) && (vHigh == fHigh);
    bool mGlobal = rHigh && vHigh && fHigh;

    float rMS = 0.0f, vppMS = 0.0f, fMS = 0.0f, gMS = 0.0f;
    updateSettleTimers(engine, rHigh, vHigh, fHigh, mGlobal, rMS, vppMS, fMS, gMS);

    ImGui::Text("Gating Verification Deck:"); ImGui::Separator(); ImGui::Spacing();
    
    auto renderTrendArrow = [](float currentVal, float lastVal) {
        float delta = currentVal - lastVal;
        if (delta > 0.0002f)       ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.3f, 1.0f), "^"); 
        else if (delta < -0.0002f) ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "v"); 
        else                       ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "-"); 
    };

    // Row 1: RMS Track 
    ImGui::Text("RMS: %.3fV >= %.3fV", signalRMS, expectedRMSThreshold); ImGui::SameLine(185);
    renderTrendArrow(signalRMS, lastRMS); ImGui::SameLine(215);
    ImGui::TextColored(rHigh ? ImVec4(0.0f, 1.0f, 0.3f, 1.0f) : ImVec4(1.0f, 0.3f, 0.3f, 1.0f), rHigh ? "T" : "F"); ImGui::SameLine(235); ImGui::TextDisabled("(%.0fms)", rMS);
    ImGui::Spacing();
    
    // Row 2: Vpp Track
    ImGui::Text("Vpp: %.3fV >= %.3fV", signalVpp, expectedVppThreshold); ImGui::SameLine(185);
    renderTrendArrow(signalVpp, lastVpp); ImGui::SameLine(215);
    ImGui::TextColored(vHigh ? ImVec4(0.0f, 1.0f, 0.3f, 1.0f) : ImVec4(1.0f, 0.3f, 0.3f, 1.0f), vHigh ? "T" : "F"); ImGui::SameLine(235); ImGui::TextDisabled("(%.0fms)", vppMS);
    ImGui::Spacing();
    
    // Row 3: Frequency Track
    ImGui::Text("Frq: %.3fw ~= %.3fw", measuredFrequency, targetFreqThreshold); ImGui::SameLine(185);
    renderTrendArrow(measuredFrequency, lastFreq); ImGui::SameLine(215);
    if (!fHigh) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "F");
    } else {
        ImVec4 targetColor = (engine.getActiveSceneMode() == 0 || engine.getActiveSceneMode() == 4) ? ImVec4(0.0f, 1.0f, 0.3f, 1.0f) : ImVec4(0.2f, 0.8f, 1.0f, 1.0f); 
        ImGui::TextColored(targetColor, "T");
    }
    ImGui::SameLine(235); ImGui::TextDisabled("(%.0fms)", fMS);
    
    ImGui::Separator(); 
    
    ImGui::Text("GLOBAL GATE TRUTH:"); ImGui::SameLine(215); 
    if (!hasConsensus) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "-");
    } else {
        if (mGlobal) ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.3f, 1.0f), "T");
        else         ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "F");
    }
    ImGui::SameLine(235); ImGui::TextDisabled("(%.0fms)", gMS);
    ImGui::EndChild();
    ImGui::SameLine(715);

    if (!engine.getPauseState()) {
        lastRMS = signalRMS;
        lastVpp = signalVpp;
        lastFreq = measuredFrequency;
    }

    // --- SUB-PANEL D: CONDENSED PLOT VIEW ---
    ImGui::BeginChild("##SubPanelOscilloscopes", ImVec2(145, 180), false, ImGuiWindowFlags_NoScrollbar);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.04f, 0.04f, 0.06f, 1.0f));
    
    ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.0f, 0.9f, 1.0f, 1.0f));
    ImGui::PlotLines("##VOscilloscope", vPlotCache.data(), (int)vPlotCache.size(), 0, nullptr, -1.5f, 1.5f, ImVec2(-1, 45));
    ImGui::PopStyleColor();
    ImVec2 vTextPos = ImGui::GetItemRectMin(); vTextPos.x += 4.0f; vTextPos.y += 2.0f;
    char vStr[32]; // FIXED: Proper 32-byte char array string allocation
    std::snprintf(vStr, sizeof(vStr), "V:%+.2f", currentSample); ImGui::GetWindowDrawList()->AddText(vTextPos, IM_COL32(255, 255, 255, 220), vStr);
    ImGui::TextDisabled("Mn:%+.2f Mx:%+.2f", vMin, vMax); 
    
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(1.0f, 0.6f, 0.0f, 1.0f));
    ImGui::PlotLines("##FOscilloscope", fPlotCache.data(), (int)fPlotCache.size(), 0, nullptr, 0.00f, 0.20f, ImVec2(-1, 45));
    ImGui::PopStyleColor();
    ImVec2 fTextPos = ImGui::GetItemRectMin(); fTextPos.x += 4.0f; fTextPos.y += 2.0f;
    char fStr[32]; // FIXED: Proper 32-byte char array string allocation
    std::snprintf(fStr, sizeof(fStr), "w:%.3f", measuredFrequency); ImGui::GetWindowDrawList()->AddText(fTextPos, IM_COL32(255, 255, 255, 220), fStr);
    ImGui::TextDisabled("Mn:%.3f Mx:%.3f", fMin, fMax); 
    
    ImGui::PopStyleColor();
    ImGui::EndChild(); 
    ImGui::End();
}
