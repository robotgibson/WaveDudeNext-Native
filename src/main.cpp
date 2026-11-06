#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "engine/WaveEngine.hpp"
#include <vector>
#include <iostream>

const int WIDTH = 1120; 
const int HEIGHT = 680;
const int PLOT_SAMPLES = 120; 

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) return 1;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_Window* window = SDL_CreateWindow("WaveGuide Native C++ Pro Studio", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); 

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 130");

    WaveEngine engine(800, 600); 
    std::vector<uint32_t> pixelBuffer(800 * 600, 0);
    
    GLuint waveTexture;
    glGenTextures(1, &waveTexture);
    glBindTexture(GL_TEXTURE_2D, waveTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    bool running = true;
    float currentSliderPhase = 0.0f;
    float currentSliderFreq = 0.08f;
    float currentSliderAmp = 0.8f;
    
    bool bitA = true;
    bool bitB = true;

    int probeX = 360;
    int probeY = 185;

    int activeComboPaletteIdx = 0;
    const char* paletteThemesList[] = { "Classic Field Tone", "Green / Magenta Field", "Monochrome Radar Glow" };

    std::vector<float> plotData(PLOT_SAMPLES, 0.0f);

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) running = false;
        }

        engine.stepSimulation();
        engine.updatePixels(pixelBuffer.data());

        if (!engine.getPauseState()) {
            float currentSample = engine.sampleAmplitude(probeX, probeY);
            for (int i = 0; i < PLOT_SAMPLES - 1; ++i) {
                plotData[i] = plotData[i + 1];
            }
            plotData[PLOT_SAMPLES - 1] = currentSample;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(280, 660), ImGuiCond_Always);
        ImGui::Begin("Wave Tuning Desk");
        
        ImGui::Text("Simulation Scene Profile:");
        if (ImGui::Button("Gates", ImVec2(55, 30))) {
            engine.resetToProceduralGates();
            currentSliderFreq = 0.08f; engine.setFrequency(0.08f);
            probeX = 360; probeY = 185; 
        }
        ImGui::SameLine();
        if (ImGui::Button("Ball Lens", ImVec2(95, 30))) {
            engine.loadSceneFromImage("");
            currentSliderFreq = 0.12f; engine.setFrequency(0.12f); 
            probeX = 580; probeY = 300; 
        }
        ImGui::SameLine();
        if (ImGui::Button("Glass Prism", ImVec2(100, 30))) {
            engine.switchToPrismScene();
            currentSliderFreq = 0.07f; engine.setFrequency(0.07f); 
            probeX = 550; probeY = 380; 
        }

        ImGui::Separator();
        
        if (!engine.getPauseState()) {
            if (ImGui::Button("Pause Execution Engine", ImVec2(-1, 30))) { engine.setPauseState(true); }
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
            if (ImGui::Button("Resume Physics Loop", ImVec2(-1, 30))) { engine.setPauseState(false); }
            ImGui::PopStyleColor();
        }

        ImGui::Separator();
        ImGui::Text("Visualization Output Layer:");
        bool currentMode = engine.getIntensityMode();
        if (ImGui::RadioButton("Amplitude (Ripples)", !currentMode)) { engine.setIntensityMode(false); }
        ImGui::SameLine();
        if (ImGui::RadioButton("Intensity (Beams)", currentMode)) { engine.setIntensityMode(true); }

        ImGui::Separator();
        ImGui::Text("Render Color Scheme:");
        if (ImGui::Combo("Theme##Palette", &activeComboPaletteIdx, paletteThemesList, IM_ARRAYSIZE(paletteThemesList))) {
            engine.setColorPalette(activeComboPaletteIdx);
        }

        ImGui::Separator();
        ImGui::Text("Global Signal Inputs:");
        if (ImGui::Checkbox("Input Channel A", &bitA) || ImGui::Checkbox("Input Channel B", &bitB)) {
            engine.toggleInputs(bitA, bitB);
        }

        ImGui::Separator();
        ImGui::Text("Laser Source Parameters:");
        if (ImGui::SliderFloat("Power (Gain)", &currentSliderAmp, 0.0f, 2.0f, "A=%.2f")) {
            engine.setAmplitude(currentSliderAmp);
        }
        if (ImGui::SliderFloat("Frequency", &currentSliderFreq, 0.04f, 0.16f, "w=%.3f")) {
            engine.setFrequency(currentSliderFreq);
        }
        if (ImGui::SliderFloat("Phase (Deg)", &currentSliderPhase, -180.0f, 180.0f, "%.1f")) {
            engine.setPhaseShift(currentSliderPhase);
        }

        ImGui::Text("XOR Gate Presets:");
        if (ImGui::Button("Standard State (0 Deg)", ImVec2(-1, 24)))     { currentSliderPhase = 0.0f;   engine.setPhaseShift(0.0f); }
        if (ImGui::Button("Optical OR Gate (-30 Deg)", ImVec2(-1, 24)))  { currentSliderPhase = -30.0f; engine.setPhaseShift(-30.0f); }
        if (ImGui::Button("Optical XOR Gate (-90 Deg)", ImVec2(-1, 24))) { currentSliderPhase = -90.0f; engine.setPhaseShift(-90.0f); }
        
        ImGui::Separator();
        ImGui::Text("Output Probe Analyzer (X:%d, Y:%d):", probeX, probeY);
        ImGui::PlotLines("##Oscilloscope", plotData.data(), PLOT_SAMPLES, 0, nullptr, -1.5f, 1.0f, ImVec2(-1, 100));

        ImGui::Separator();
        ImGui::Text("AND Gate Evaluation Table:");
        ImGui::Text("A=%d, B=%d", bitA, bitB);

        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 45);
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), "Performance: %.1f FPS (%.3f ms/frame)", 
            io.Framerate, 1000.0f / io.Framerate);

        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(300, 10), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(810, 660), ImGuiCond_Always);
        ImGui::Begin("Simulation Monitor");
        
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        
        glBindTexture(GL_TEXTURE_2D, waveTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer.data());
        ImGui::Image((ImTextureID)(intptr_t)waveTexture, ImVec2(800, 600));
        
        if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            ImVec2 mousePos = ImGui::GetMousePos();
            int clickedX = static_cast<int>(mousePos.x - canvasPos.x);
            int clickedY = static_cast<int>(mousePos.y - canvasPos.y);
            
            if (clickedX >= 10 && clickedX < 790 && clickedY >= 10 && clickedY < 590) {
                probeX = clickedX;
                probeY = clickedY;
            }
        }
        
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 markerScreenPos = ImVec2(canvasPos.x + probeX, canvasPos.y + probeY);
        drawList->AddCircle(markerScreenPos, 7.0f, IM_COL32(0, 255, 255, 255), 16, 2.5f);
        drawList->AddCircleFilled(markerScreenPos, 2.5f, IM_COL32(255, 255, 255, 255));
        
        ImGui::End();

        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    glDeleteTextures(1, &waveTexture);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
