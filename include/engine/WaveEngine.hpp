#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <memory>

// Base structural class for all modular optical canvas parts
class OpticalPart {
protected:
    std::string name;
    int posX;
    int posY;

public:
    OpticalPart(const std::string& n, int x, int y) : name(n), posX(x), posY(y) {}
    virtual ~OpticalPart() = default;
    
    // Derived components override this to write their specific refractive index fingerprints
    virtual void burnIntoMap(std::vector<float>& indexMap, int width, int height) = 0;
};

// Specialized XOR Gate Component Module
class XorGatePart : public OpticalPart {
public:
    XorGatePart(const std::string& n, int x, int y) : OpticalPart(n, x, y) {}
    void burnIntoMap(std::vector<float>& indexMap, int width, int height) override;
};

// Specialized AND Gate Component Module (Requires a constant reference source track)
class AndGatePart : public OpticalPart {
public:
    AndGatePart(const std::string& n, int x, int y) : OpticalPart(n, x, y) {}
    void burnIntoMap(std::vector<float>& indexMap, int width, int height) override;
};

// Core Engine Class managing the collective array pipelines
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
    float phaseOffset = 0.0f;
    float frequency = 0.15f;

public:
    WaveEngine(int w, int h);
    ~WaveEngine() = default;

    void addOpticalPart(std::unique_ptr<OpticalPart> part);
    void initSimulationSpace();
    void stepSimulation();
    void setPhaseShift(float degrees);
    void updatePixels(uint32_t* pixelBuffer);
};
