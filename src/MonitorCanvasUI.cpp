#include "imgui.h"
#include <SDL2/SDL_opengl.h>
#include "engine/WaveEngine.hpp"
#include <vector>
#include <cmath>

void renderMonitorCanvas(WaveEngine& engine, GLuint texID, std::vector<uint32_t>& buffer, int& pX, int& pY) {
    ImGui::SetNextWindowPos(ImVec2(320, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(810, 700), ImGuiCond_Always);
    ImGui::Begin("Simulation Monitor", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    auto& nodes = engine.getCustomSourceNodes();
    int gX = engine.getGlobalOriginX();
    int gY = engine.getGlobalOriginY();
    
    // Render left-aligned metrics text field cleanly across the top header row
    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "Performance Metrics: %.1f FPS (%.2f ms/frame)", 
        ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
    
    ImGui::Spacing();
    ImGui::Image((ImTextureID)(intptr_t)texID, ImVec2(800, 600));
    
    if (ImGui::IsItemHovered()) {
        ImVec2 mousePos = ImGui::GetMousePos();
        int clickedX = static_cast<int>(mousePos.x - canvasPos.x);
        int clickedY = static_cast<int>(mousePos.y - (canvasPos.y + 24)); 

        if (clickedX >= 5 && clickedX < 795 && clickedY >= 5 && clickedY < 595) {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                bool clickedNode = false;
                for (size_t i = 0; i < nodes.size(); ++i) {
                    int absX = gX + nodes[i].offsetX;
                    int absY = gY + nodes[i].offsetY;
                    if (std::abs(absX - clickedX) < 8 && std::abs(absY - clickedY) < 8) {
                        engine.setSelectedNodeIndex(static_cast<int>(i)); clickedNode = true; break;
                    }
                }
                if (!clickedNode) { pX = clickedX; pY = clickedY; }
            }
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                engine.addIndependentSourceNode(clickedX, clickedY);
            }
        }
    }
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 contentOffsetPos = ImVec2(canvasPos.x, canvasPos.y + 24); 
    
    ImVec2 markerScreenPos = ImVec2(contentOffsetPos.x + pX, contentOffsetPos.y + pY);
    drawList->AddCircle(markerScreenPos, 7.0f, IM_COL32(0, 255, 255, 255), 16, 2.5f);
    drawList->AddCircleFilled(markerScreenPos, 2.5f, IM_COL32(255, 255, 255, 255));

    if (gX > 0 && gX < 800 && gY > 0 && gY < 600) {
        ImVec2 globalOriginScreenPos = ImVec2(contentOffsetPos.x + gX, contentOffsetPos.y + gY);
        drawList->AddRect(ImVec2(globalOriginScreenPos.x - 5, globalOriginScreenPos.y - 5),
                          ImVec2(globalOriginScreenPos.x + 5, globalOriginScreenPos.y + 5), IM_COL32(255, 255, 255, 180), 0, 0, 1.5f);
        drawList->AddLine(ImVec2(globalOriginScreenPos.x - 12, globalOriginScreenPos.y), ImVec2(globalOriginScreenPos.x + 12, globalOriginScreenPos.y), IM_COL32(255, 255, 255, 140), 1.0f);
        drawList->AddLine(ImVec2(globalOriginScreenPos.x, globalOriginScreenPos.y - 12), ImVec2(globalOriginScreenPos.x, globalOriginScreenPos.y + 12), IM_COL32(255, 255, 255, 140), 1.0f);
    }

    int activeFocusIdx = engine.getSelectedNodeIndex();
    for (size_t i = 0; i < nodes.size(); ++i) {
        int absoluteX = gX + nodes[i].offsetX;
        int absoluteY = gY + nodes[i].offsetY;
        
        if (absoluteX < 0 || absoluteX >= 800 || absoluteY < 0 || absoluteY >= 600) continue;
        ImVec2 sourceScreenPos = ImVec2(contentOffsetPos.x + absoluteX, contentOffsetPos.y + absoluteY);
        
        if (static_cast<int>(i) == activeFocusIdx) {
            drawList->AddCircle(sourceScreenPos, 12.0f, IM_COL32(255, 215, 0, 255), 16, 2.5f);
            drawList->AddCircle(sourceScreenPos, 8.0f, IM_COL32(255, 65, 45, 255), 16, 2.0f);
        } else {
            drawList->AddCircle(sourceScreenPos, 8.0f, nodes[i].isActive ? IM_COL32(255, 45, 45, 180) : IM_COL32(100, 100, 100, 150), 16, 1.5f);
        }
        drawList->AddCircleFilled(sourceScreenPos, 3.0f, nodes[i].isActive ? IM_COL32(255, 120, 120, 255) : IM_COL32(150, 150, 150, 255));
    }
    
    ImGui::End();
}

