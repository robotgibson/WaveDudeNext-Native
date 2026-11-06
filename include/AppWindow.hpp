#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <string>

class AppWindow {
private:
    SDL_Window* window = nullptr;
    SDL_GLContext glContext = nullptr;
    int width;
    int height;

public:
    AppWindow(const std::string& title, int w, int h);
    ~AppWindow();

    bool init();
    void clear(float r, float g, float b, float a);
    void swap();
    SDL_Window* getNativeWindow() { return window; }
};
