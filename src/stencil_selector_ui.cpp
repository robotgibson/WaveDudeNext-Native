#include "stencil_selector_ui.h"
#include "imgui.h"

void renderStencilPrecisionControls(WaveEngine& engine) {
    ImGui::Separator();
    ImGui::Text("FDTD Stencil Solvers:");
    
    const char* precisionModes[] = { "2nd-Order (Low-Fi/Dispersion)", "4th-Order (High-Accuracy Max)" };
    int currentModeIdx = static_cast<int>(engine.getStencilPrecision());
    
    ImGui::SetNextItemWidth(-1);
    if (ImGui::Combo("##StencilSelect", &currentModeIdx, precisionModes, IM_ARRAYSIZE(precisionModes))) {
        engine.setStencilPrecision(static_cast<WaveEngine::StencilPrecision>(currentModeIdx));
    }
}

