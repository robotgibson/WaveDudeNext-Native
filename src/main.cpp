#include "AppWindow.hpp"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "engine/WaveEngine.hpp"
#include "zoom_pip_ui.h" 
#include <vector>
#include <cstring>

void renderTuningDesk(WaveEngine& engine, char* pathBuffer, size_t bufferSize, 
                      float& sliderAmp, float& sliderFreq, float& sliderPhase,
                      bool& bitA, bool& bitB, int& paletteIdx, int& pX, int& pY);

void renderMonitorCanvas(WaveEngine& engine, GLuint texID, std::vector<uint32_t>& buffer, int& pX, int& pY);

// Declare prototype link to trigger the configuration parser setup manually
void syncUIPropertiesFromJSON(WaveEngine& engine, int targetSceneID, int& pX, int& pY);

int main(int argc, char* argv[]) {
    AppWindow app("WaveGuide Native C++ Pro Studio", 1140, 720);
    if (!app.init()) return 1;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(app.getNativeWindow(), SDL_GL_GetCurrentContext());
    ImGui_ImplOpenGL3_Init("#version 130");

    WaveEngine engine(800, 600); 
    std::vector<uint32_t> pixelBuffer(800 * 600, 0);
    
    GLuint waveTexture;
    glGenTextures(1, &waveTexture);
    glBindTexture(GL_TEXTURE_2D, waveTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    bool running = true;
    float sAmp = 0.8f, sFreq = 0.08f, sPhase = 0.0f;
    bool bitA = true, bitB = true;
    int paletteIdx = 0;
    
    // Launch placeholders that will be immediately updated from the JSON payload below
    int pX = 0, pY = 0; 
    char pathBuffer[128] = "double_slit_experiment.png";

    // FIXED: Force immediate JSON profile alignment pass for the AND Gate (ID: 0) on launch
    engine.switchToAndGate();
    syncUIPropertiesFromJSON(engine, 0, pX, pY);

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) running = false;
        }

        app.clear(0.10f, 0.10f, 0.12f, 1.0f);

        engine.stepSimulation();
        engine.updatePixels(pixelBuffer.data());

        glBindTexture(GL_TEXTURE_2D, waveTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer.data());

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        renderTuningDesk(engine, pathBuffer, sizeof(pathBuffer), sAmp, sFreq, sPhase, bitA, bitB, paletteIdx, pX, pY);
        renderMonitorCanvas(engine, waveTexture, pixelBuffer, pX, pY);
        renderAnalyticsFooterPanel(engine, pixelBuffer, pX, pY); 

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        app.swap();
    }

    glDeleteTextures(1, &waveTexture);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    return 0;
}
