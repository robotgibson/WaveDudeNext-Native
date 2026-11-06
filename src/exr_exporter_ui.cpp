#include "exr_exporter_ui.h"
#include "exr_exporter.h"
#include "imgui.h"

void renderEXRExportLayout(WaveEngine& engine) {
    ImGui::Separator();
    ImGui::Text("Photonic Data Export");

    static char exrFilename[128] = "optical_logic_snapshot.exr";
    ImGui::InputText("Filename##EXR", exrFilename, IM_ARRAYSIZE(exrFilename));

    if (ImGui::Button("Export Raw EXR Grid")) {
        float* fieldData = engine.getPrimaryFieldData();
        if (fieldData != nullptr) {
            bool success = ExportSimulationToEXR(
                exrFilename, 
                fieldData, 
                engine.getGridWidth(), 
                engine.getGridHeight()
            );
            
            if (success) {
                ImGui::OpenPopup("EXR Export Success");
            } else {
                ImGui::OpenPopup("EXR Export Failure");
            }
        }
    }

    // Interactive Status Feedback Modals
    if (ImGui::BeginPopupModal("EXR Export Success", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Success! Raw 32-bit float array dumped cleanly to disk.");
        ImGui::Text("Path: %s", exrFilename);
        ImGui::Separator();
        if (ImGui::Button("Close", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("EXR Export Failure", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Error: Could not save the EXR container.");
        ImGui::Text("Verify target folder directory exists and has write permissions.");
        ImGui::Separator();
        if (ImGui::Button("Close", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

