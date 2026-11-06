#include "imgui.h"
#include <SDL2/SDL_opengl.h>
#include "engine/WaveEngine.hpp"
#include <vector>

void renderMonitorCanvas(WaveEngine& engine, GLuint texID, std::vector<uint32_t>& buffer, int& pX, int& pY) {
    ImGui::SetNextWindowPos(ImVec2(320, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(810, 700), ImGuiCond_Always);
    ImGui::Begin("Simulation Monitor", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    auto& nodes = engine.getCustomSourceNodes();
    
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
    ImGui::Image((ImTextureID)(intptr_t)texID, ImVec2(800, 600));
    
    if (ImGui::IsItemHovered()) {
        ImVec2 mousePos = ImGui::GetMousePos();
        int clickedX = static_cast<int>(mousePos.x - canvasPos.x);
        int clickedY = static_cast<int>(mousePos.y - canvasPos.y);

        if (clickedX >= 5 && clickedX < 795 && clickedY >= 5 && clickedY < 595) {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                // If a left-click falls directly onto an existing source pin, focus it instead of moving the probe
                bool clickedNode = false;
                for (size_t i = 0; i < nodes.size(); ++i) {
                    if (std::abs(nodes[i].x - clickedX) < 8 && std::abs(nodes[i].y - clickedY) < 8) {
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
    
    // Render Cyan Testing Reticle Circle 
    ImVec2 markerScreenPos = ImVec2(canvasPos.x + pX, canvasPos.y + pY);
    drawList->AddCircle(markerScreenPos, 7.0f, IM_COL32(0, 255, 255, 255), 16, 2.5f);
    drawList->AddCircleFilled(markerScreenPos, 2.5f, IM_COL32(255, 255, 255, 255));

    // Render Emitter Pins
    int activeFocusIdx = engine.getSelectedNodeIndex();
    for (size_t i = 0; i < nodes.size(); ++i) {
        ImVec2 sourceScreenPos = ImVec2(canvasPos.x + nodes[i].x, canvasPos.y + nodes[i].y);
        
        if (static_cast<int>(i) == activeFocusIdx) {
            // Draws a glowing golden halo circle around the active sidebar focused element
            drawList->AddCircle(sourceScreenPos, 12.0f, IM_COL32(255, 215, 0, 255), 16, 2.5f);
            drawList->AddCircle(sourceScreenPos, 8.0f, IM_COL32(255, 65, 45, 255), 16, 2.0f);
        } else {
            drawList->AddCircle(sourceScreenPos, 8.0f, nodes[i].isActive ? IM_COL32(255, 45, 45, 180) : IM_COL32(100, 100, 100, 150), 16, 1.5f);
        }
        drawList->AddCircleFilled(sourceScreenPos, 3.0f, nodes[i].isActive ? IM_COL32(255, 120, 120, 255) : IM_COL32(150, 150, 150, 255));
    }
    
    ImGui::End();
}
