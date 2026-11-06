#pragma once
#include "imgui.h"
#include <algorithm>
#include <type_traits>

template<typename T>
inline bool PrecisionSlider(const char* label, T* value, T minVal, T maxVal, T step, const char* format = nullptr) {
    ImGui::PushID(label);
    bool changed = false;

    // 1. Render Left Minus Arrow Button
    if (ImGui::Button("<", ImVec2(24, 22))) {
        *value = std::max(minVal, static_cast<T>(*value - step));
        changed = true;
    }
    
    // 2. Render Main Center Horizontal Slider Track
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 110);
    
    if constexpr (std::is_same_v<T, int>) {
        if (ImGui::SliderInt("##Slider", value, minVal, maxVal, format ? format : "%d")) changed = true;
    } else {
        if (ImGui::SliderFloat("##Slider", value, minVal, maxVal, format ? format : "%.3f")) changed = true;
    }
    
    // 3. Render Right Plus Arrow Button
    ImGui::SameLine();
    if (ImGui::Button(">", ImVec2(24, 22))) {
        *value = std::min(maxVal, static_cast<T>(*value + step));
        changed = true;
    }

    // 4. Render Label text field cleanly right-aligned
    ImGui::SameLine();
    ImGui::Text("%s", label);

    ImGui::PopID();
    return changed;
}

// FIXED: Force the compiler to generate concrete, linkable binary definitions for both types globally
template bool PrecisionSlider<int>(const char*, int*, int, int, int, const char*);
template bool PrecisionSlider<float>(const char*, float*, float, float, float, const char*);
