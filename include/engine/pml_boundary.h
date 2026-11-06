#pragma once
#include <vector>

// Procedurally calculates and populates the quadratic damping layer array profile near grid edges
void initializePMLBoundaryLayers(std::vector<float>& dampingMap, int width, int height, int pmlThickness = 20);

