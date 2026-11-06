#include "engine/WaveEngine.hpp"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <SDL2/SDL.h>

WaveEngine::WaveEngine(int w, int h) : width(w), height(h),
    gridCurrent(w * h, 0.0f), gridPrevious(w * h, 0.0f), gridIntensity(w * h, 0.0f),
    refractiveIndexMap(w * h, 1.0f), dampingMap(w * h, 0.0f)
{
    resetToProceduralGates();
}

void WaveEngine::addOpticalPart(std::unique_ptr<OpticalPart> part) {
    partsList.push_back(std::move(part));
}

void WaveEngine::clearGridFields() {
    std::fill(gridCurrent.begin(), gridCurrent.end(), 0.0f);
    std::fill(gridPrevious.begin(), gridPrevious.end(), 0.0f);
    std::fill(gridIntensity.begin(), gridIntensity.end(), 0.0f);
    for (auto& node : customSourceNodes) { node.phaseValue = 0.0f; }
}

void WaveEngine::addIndependentSourceNode(int x, int y) {
    int cx = std::clamp(x, 5, width - 5);
    int cy = std::clamp(y, 5, height - 5);
    
    for (const auto& node : customSourceNodes) {
        if (std::abs(node.x - cx) < 4 && std::abs(node.y - cy) < 4) return;
    }

    // FIXED: Strict cap constraint enforced at 16 elements maximum layout
    if (customSourceNodes.size() < 16) {
        CustomLaserNode node;
        node.x = cx;
        node.y = cy;
        node.frequencyOverride = globalFrequency;
        node.amplitudeScale = 1.0f;
        node.isActive = true;
        customSourceNodes.push_back(node);
        selectedNodeIndex = static_cast<int>(customSourceNodes.size()) - 1; 
    }
}

void WaveEngine::clearIndependentSourceNodes() {
    customSourceNodes.clear(); selectedNodeIndex = -1;
}

void WaveEngine::resetToProceduralGates() {
    activeSceneMode = 0; partsList.clear();
    clearIndependentSourceNodes();
    
    addIndependentSourceNode(280, 165); 
    addIndependentSourceNode(280, 235); 
    addIndependentSourceNode(280, 444); 
    addIndependentSourceNode(280, 460); 
    addIndependentSourceNode(280, 476); 
    
    if (customSourceNodes.size() >= 4) {
        customSourceNodes[3].phaseOffsetDeg = 180.0f; 
        customSourceNodes[3].amplitudeScale = 0.625f; 
    }

    initSimulationSpace();
    clearGridFields();
}

void WaveEngine::switchToPrismScene() {
    activeSceneMode = 2; partsList.clear();
    clearIndependentSourceNodes();
    
    addIndependentSourceNode(60, height / 2 + 60); 
    addOpticalPart(std::make_unique<PrismPart>("Glass Prism 1", 450, 300, 160, 0.0f));
    
    initSimulationSpace();
    clearGridFields();
}

bool WaveEngine::loadSceneFromImage(const std::string& filePath) {
    SDL_Surface* loadedSurface = SDL_LoadBMP(filePath.c_str());
    if (!loadedSurface) {
        std::cerr << "CRITICAL: Native BMP loading failed for path: " << filePath << std::endl;
        return false;
    }
    SDL_Surface* img = SDL_ConvertSurfaceFormat(loadedSurface, SDL_PIXELFORMAT_RGB24, 0);
    SDL_FreeSurface(loadedSurface);
    if (!img) return false;

    activeSceneMode = 1; partsList.clear(); clearGridFields();
    std::fill(refractiveIndexMap.begin(), refractiveIndexMap.end(), 1.0f);
    std::fill(dampingMap.begin(), dampingMap.end(), 0.0f);
    clearIndependentSourceNodes();

    int matchW = std::min(width, img->w);
    int matchH = std::min(height, img->h);
    uint8_t* pixels = static_cast<uint8_t*>(img->pixels);

    for (int y = 0; y < matchH; ++y) {
        for (int x = 0; x < matchW; ++x) {
            int gridIdx = y * width + x;
            int imgIdx = (y * img->pitch) + (x * 3);
            uint8_t r = pixels[imgIdx + 0]; uint8_t g = pixels[imgIdx + 1]; uint8_t b = pixels[imgIdx + 2];
            if (r > 0) refractiveIndexMap[gridIdx] = 1.0f + (static_cast<float>(r) / 255.0f);
            if (g > 128) { addIndependentSourceNode(x, y); } 
            if (b > 0) dampingMap[gridIdx] = static_cast<float>(b) / 255.0f * 0.7f;
        }
    }
    SDL_FreeSurface(img);

    int border = 40;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int dist = 0;
            if (x < border) dist = std::max(dist, border - x);
            if (x > width - border) dist = std::max(dist, x - (width - border));
            if (y < border) dist = std::max(dist, border - y);
            if (y > height - border) dist = std::max(dist, y - (height - border));
            if (dist > 0) {
                float normDist = static_cast<float>(dist) / border;
                dampingMap[y * width + x] = std::max(dampingMap[y * width + x], 0.15f * (normDist * normDist)); 
            }
        }
    }
    return true;
}

void WaveEngine::exportSurfaceToPNG(const std::string& filename, const std::vector<uint8_t>& rleBuffer) {
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormatFrom(
        const_cast<uint8_t*>(rleBuffer.data()), width, height, 24, width * 3, SDL_PIXELFORMAT_RGB24);
    if (surf) {
        std::string bmpName = filename + ".bmp";
        SDL_SaveBMP(surf, bmpName.c_str());
        SDL_FreeSurface(surf);
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
            int idx = (y * width + x) * 3;
            int dx = x - lensX; int dy = y - lensY;
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

void WaveEngine::initSimulationSpace() {
    std::fill(refractiveIndexMap.begin(), refractiveIndexMap.end(), 1.0f);
    std::fill(dampingMap.begin(), dampingMap.end(), 0.0f);
    
    if (partsList.empty()) {
        if (activeSceneMode == 0) {
            addOpticalPart(std::make_unique<XorGatePart>("XOR Gate 1", 450, 200));
            addOpticalPart(std::make_unique<AndGatePart>("AND Gate 2", 450, 460));
        } else if (activeSceneMode == 2) {
            addOpticalPart(std::make_unique<PrismPart>("Glass Prism 1", 450, 300, 160, 0.0f));
        }
    }

    for (const auto& part : partsList) { part->burnIntoMap(refractiveIndexMap, width, height); }
    
    int border = 40;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int dist = 0;
            if (x < border) dist = std::max(dist, border - x);
            if (x > width - border) dist = std::max(dist, x - (width - border));
            if (y < border) dist = std::max(dist, border - y);
            if (y > height - border) dist = std::max(dist, y - (height - border));
            if (dist > 0) {
                float normDist = static_cast<float>(dist) / border;
                dampingMap[y * width + x] = std::max(dampingMap[y * width + x], 0.15f * (normDist * normDist)); 
            }
        }
    }
    if (activeSceneMode == 0) {
        int barrierX = 273; 
        for (int y = 50; y < 550; ++y) {
            for (int x = barrierX - 15; x <= barrierX; ++x) { dampingMap[y * width + x] = 0.8f; }
        }
    }
}
