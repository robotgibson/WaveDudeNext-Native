#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <memory>

class OpticalPart {
protected:
    std::string name;
    int posX;
    int posY;

public:
    OpticalPart(const std::string& n, int x, int y) : name(n), posX(x), posY(y) {}
    virtual ~OpticalPart() = default;
    virtual void burnIntoMap(std::vector<float>& indexMap, int width, int height) = 0;
};

class XorGatePart : public OpticalPart {
public:
    XorGatePart(const std::string& n, int x, int y) : OpticalPart(n, x, y) {}
    void burnIntoMap(std::vector<float>& indexMap, int width, int height) override;
};

class AndGatePart : public OpticalPart {
public:
    AndGatePart(const std::string& n, int x, int y) : OpticalPart(n, x, y) {}
    void burnIntoMap(std::vector<float>& indexMap, int width, int height) override;
};

class PrismPart : public OpticalPart {
private:
    int baseSize;
public:
    PrismPart(const std::string& n, int x, int y, int size) : OpticalPart(n, x, y), baseSize(size) {}
    void burnIntoMap(std::vector<float>& indexMap, int width, int height) override;
};

class WaveEngine {
private:
    int width;
    int height;
    
    std::vector<float> gridCurrent;
    std::vector<float> gridPrevious;
    std::vector<float> refractiveIndexMap;
    std::vector<float> dampingMap;

    std::vector<std::unique_ptr<OpticalPart>> partsList;

    float phaseA = 0.0f;
    float phaseB = 0.0f;
    float phaseRef = 0.0f;
    float phaseOffset = 0.0f;
    float frequency = 0.08f;
    float sourceAmplitude = 0.8f;

    bool inputA_Active = true;
    bool inputB_Active = true;
    bool isPaused = false;
    
    int activeSceneMode = 0; 
    int activeColorPalette = 0; 
    int sourceX = 280;
    int sourceY1 = 165;
    int sourceY2 = 235;

public:
    WaveEngine(int w, int h);
    ~WaveEngine() = default;

    void addOpticalPart(std::unique_ptr<OpticalPart> part);
    void initSimulationSpace();
    void resetToProceduralGates();
    bool loadSceneFromImage(const std::string& filePath);
    void switchToPrismScene();
    void clearGridFields();

    void stepSimulation();
    void setPhaseShift(float degrees);
    void setFrequency(float freq);
    void setAmplitude(float amp);
    void setColorPalette(int paletteID);
    
    void setPauseState(bool pause) { isPaused = pause; }
    bool getPauseState() const { return isPaused; }

    void toggleInputs(bool bitA, bool bitB);
    float sampleAmplitude(int x, int y) const;
    void updatePixels(uint32_t* pixelBuffer);
};
