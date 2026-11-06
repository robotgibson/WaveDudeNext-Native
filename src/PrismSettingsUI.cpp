#include "imgui.h"
#include "engine/WaveEngine.hpp"
#include "UIRegistry.hpp" // FIXED: Included shared precision slider template maps
#include <algorithm>

void renderPrismSettings(WaveEngine& engine) {
    if (engine.getActiveSceneMode() != 2) return;

    PrismPart* prism = static_cast<PrismPart*>(engine.getPrismPart());
    if (prism) {
        ImGui::Spacing();
        if (ImGui::CollapsingHeader("Prism Geometric Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            int posX = prism->getX(); int posY = prism->getY(); int size = prism->getSize(); float rot = prism->getRotation();
            bool changed = false;
            
            if (PrecisionSlider<int>("Prism X", &posX, 50, 750, 1))     { prism->setLocation(posX, posY); changed = true; }
            if (PrecisionSlider<int>("Prism Y", &posY, 50, 550, 1))     { prism->setLocation(posX, posY); changed = true; }
            if (PrecisionSlider<int>("Radius",  &size, 40, 300, 1))     { prism->setSize(size); changed = true; }
            if (PrecisionSlider<float>("Rotate", &rot, -180.0f, 180.0f, 0.5f, "%.1f Deg")) { prism->setRotation(rot); changed = true; }
            
            if (changed) { engine.initSimulationSpace(); }
        }
    }
}
