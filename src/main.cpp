#include <SDL2/SDL.h>
#include "engine/WaveEngine.hpp"
#include <iostream>
#include <vector>

const int WIDTH = 800;
const int HEIGHT = 600;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL Initialization Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("WaveDude Native C++ Engine", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    WaveEngine engine(WIDTH, HEIGHT);
    std::vector<uint32_t> pixelBuffer(WIDTH * HEIGHT, 0);
    
    bool running = true;
    SDL_Event event;

    std::cout << "Controls:\n [1] 0 Deg Shift (Balanced Crossover)\n [2] -30 Deg Shift ('Neat' OR Gate)\n [3] -90 Deg Shift (XOR Gate)\n";

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_1:
                        std::cout << "Switching to standard state (0 deg phase shift)\n";
                        engine.setPhaseShift(0.0f);
                        break;
                    case SDLK_2:
                        std::cout << "Switching to 'Neat' OR Gate (-30 deg phase shift)\n";
                        engine.setPhaseShift(-30.0f);
                        break;
                    case SDLK_3:
                        std::cout << "Switching to Exclusive OR (XOR) Gate (-90 deg phase shift)\n";
                        engine.setPhaseShift(-90.0f);
                        break;
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                }
            }
        }

        engine.stepSimulation();
        engine.updatePixels(pixelBuffer.data());

        SDL_UpdateTexture(texture, nullptr, pixelBuffer.data(), WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
