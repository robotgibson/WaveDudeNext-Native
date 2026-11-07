#pragma once
#include "engine/WaveEngine.hpp"
#include <vector>
#include <cstdint>

// Renders the wide analytics footer row containing both Fatbits and Probe analysis benches side-by-side
void renderAnalyticsFooterPanel(WaveEngine& engine, const std::vector<uint32_t>& pixelBuffer, int& pX, int& pY);

