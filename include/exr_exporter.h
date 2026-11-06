#pragma once
#include <string>

/**
 * Saves a 2D float simulation grid to a single-channel 32-bit float OpenEXR file.
 * 
 * @param filename   Target path (e.g., "output_frame_0100.exr")
 * @param gridData   Pointer to the contiguous raw float buffer from the FDTD engine
 * @param width      Width of the simulation matrix in pixels
 * @param height     Height of the simulation matrix in pixels
 * @return           True if successful, false otherwise
 */
bool ExportSimulationToEXR(const std::string& filename, const float* gridData, int width, int height);
