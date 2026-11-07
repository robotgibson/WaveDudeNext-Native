#include "imgui.h"
#include <SDL2/SDL_opengl.h>
#include "engine/WaveEngine.hpp"
#include <vector>
#include <cmath>

void renderMonitorCanvas(WaveEngine& engine, GLuint texID, std::vector<uint32_t>& buffer, int& pX, int& pY) {
    ImGui::SetNextWindowPos(ImVec2(320, 10), ImGuiCond_Always);
    // FIXED: Shrunk down height matrix from 700 to 525 to clear layout tracking slots for the footer panel deck
    ImGui::SetNextWindowSize(ImVec2(810, 525), ImGuiCond_Always);
    ImGui::Begin("Simulation Monitor", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    auto& nodes = engine.getCustomSourceNodes();
    int gX = engine.getGlobalOriginX();
    int gY = engine.getGlobalOriginY();
    
    ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "Performance Metrics: %.1f FPS (%.2f ms/frame)", 
        ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
    
    ImGui::Spacing();
    // Proportional downscale to map smoothly inside the shortened window canvas guidelines
    ImGui::Image((ImTextureID)(intptr_t)texID, ImVec2(800, 460));
    
    if (ImGui::IsItemHovered()) {
        ImVec2 mousePos = ImGui::GetMousePos();
        int clickedX = static_cast<int>(mousePos.x - canvasPos.x);
        int clickedY = static_cast<int>((mousePos.y - (canvasPos.y + 24)) * (600.0f / 460.0f)); 

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
    
    auto toScreen = [&](int worldX, int worldY) {
        return ImVec2(contentOffsetPos.x + worldX, contentOffsetPos.y + worldY * (460.0f / 600.0f));
    };

    ImVec2 markerScreenPos = toScreen(pX, pY);
    drawList->AddCircle(markerScreenPos, 7.0f, IM_COL32(0, 255, 255, 255), 16, 2.5f);
    drawList->AddCircleFilled(markerScreenPos, 2.5f, IM_COL32(255, 255, 255, 255));

    if (gX > 0 && gX < 800 && gY > 0 && gY < 600) {
        ImVec2 globalOriginScreenPos = toScreen(gX, gY);
        drawList->AddRect(ImVec2(globalOriginScreenPos.x - 5, globalOriginScreenPos.y - 5),
                          ImVec2(globalOriginScreenPos.x + 5, globalOriginScreenPos.y + 5), IM_COL32(255, 255, 255, 180), 0, 0, 1.5f);
    }

    int activeFocusIdx = engine.getSelectedNodeIndex();
    for (size_t i = 0; i < nodes.size(); ++i) {
        int absoluteX = gX + nodes[i].offsetX;
        int absoluteY = gY + nodes[i].offsetY;
        if (absoluteX < 0 || absoluteX >= 800 || absoluteY < 0 || absoluteY >= 600) continue;
        ImVec2 sourceScreenPos = toScreen(absoluteX, absoluteY);
        
        if (static_cast<int>(i) == activeFocusIdx) {
            drawList->AddCircle(sourceScreenPos, 12.0f, IM_COL32(255, 215, 0, 255), 16, 2.5f);
        } else {
            drawList->AddCircle(sourceScreenPos, 8.0f, nodes[i].isActive ? IM_COL32(255, 45, 45, 180) : IM_COL32(100, 100, 100, 150), 16, 1.5f);
        }
    }
    ImGui::End();
}

