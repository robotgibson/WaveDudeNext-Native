#include "engine/WaveEngine.hpp"
#include <SDL2/SDL_image.h>
#include <algorithm>
#include <iostream>

bool WaveEngine::loadSceneFromImage(const std::string& filePath) {
    SDL_Surface* loadedSurface = IMG_Load(filePath.c_str());
    if (!loadedSurface) {
        std::cerr << "CRITICAL: Compressed PNG loading failed for path: " << filePath << std::endl;
        return false;
    }
    SDL_Surface* img = SDL_ConvertSurfaceFormat(loadedSurface, SDL_PIXELFORMAT_RGB24, 0);
    SDL_FreeSurface(loadedSurface);
    if (!img) return false;

    activeSceneMode = 1; partsList.clear(); clearGridFields();
    std::fill(refractiveIndexMap.begin(), refractiveIndexMap.end(), 1.0f);
    std::fill(dampingMap.begin(), dampingMap.end(), 0.0f);
    setGlobalOrigin(0, 0);
    clearIndependentSourceNodes();

    int matchW = std::min(width, img->w); int matchH = std::min(height, img->h);
    uint8_t* pixels = static_cast<uint8_t*>(img->pixels);

    for (int y = 0; y < matchH; ++y) {
        for (int x = 0; x < matchW; ++x) {
            int gridIdx = y * width + x; int imgIdx = (y * img->pitch) + (x * 3);
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
