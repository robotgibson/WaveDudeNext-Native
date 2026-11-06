#include "engine/WaveEngine.hpp"
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>
#include <cmath>

void WaveEngine::exportSurfaceToPNG(const std::string& filename, const std::vector<uint8_t>& rleBuffer) {
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormatFrom(
        const_cast<uint8_t*>(rleBuffer.data()), width, height, 24, width * 3, SDL_PIXELFORMAT_RGB24);
    if (surf) {
        std::string pngName = filename + ".png";
        IMG_SavePNG(surf, pngName.c_str());
        SDL_FreeSurface(surf);
        std::cout << "SUCCESS: Native SDL Compressed PNG Output -> " << pngName << std::endl;
    }
}

void WaveEngine::generateNativeDoubleSlitPNG() {
    std::vector<uint8_t> buffer(width * height * 3, 0);
    int barrierX = 200; int slitSize = 24; int slitSpacing = 48; int centerY = height / 2;
    int topSlitStart = centerY - (slitSpacing / 2) - (slitSize / 2); int topSlitEnd = topSlitStart + slitSize;
    int botSlitStart = centerY + (slitSpacing / 2) - (slitSize / 2); int botSlitEnd = botSlitStart + slitSize;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 3;
            if (x >= barrierX && x <= barrierX + 15) {
                if (!((y >= topSlitStart && y <= topSlitEnd) || (y >= botSlitStart && y <= botSlitEnd))) {
                    buffer[idx + 2] = 240;
                }
            }
        }
    }
    exportSurfaceToPNG("double_slit_experiment", buffer);
}

void WaveEngine::generateNativeBallLensPNG() {
    std::vector<uint8_t> buffer(width * height * 3, 0);
    int lensX = 400; int lensY = 300; int radius = 90;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 3; int dx = x - lensX; int dy = y - lensY;
            if ((dx * dx + dy * dy) <= (radius * radius)) { buffer[idx + 0] = 140; }
        }
    }
    exportSurfaceToPNG("focusing_ball_lens", buffer);
}

void WaveEngine::generateNativeFiberNetworkPNG() {
    std::vector<uint8_t> buffer(width * height * 3, 0);
    int fiberThick = 12;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 3;
            if (x >= 40 && x < 250) { if (std::abs(y - 300) <= fiberThick / 2) buffer[idx + 0] = 165; }
            if (x >= 250 && x < 650) {
                float dx = x - 250;
                int trackTopY = 300 - static_cast<int>(0.0007f * dx * dx + 0.15f * dx);
                int trackBotY = 300 + static_cast<int>(0.0007f * dx * dx + 0.15f * dx);
                if (std::abs(y - trackTopY) <= fiberThick / 2 || std::abs(y - trackBotY) <= fiberThick / 2) buffer[idx + 0] = 165;
            }
        }
    }
    exportSurfaceToPNG("fiber_splitter_network", buffer);
}
