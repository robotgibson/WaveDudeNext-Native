#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <memory>
#include <utility>

class OpticalPart {
protected:
    std::string name;
    int posX;
    int posY;

public:
    OpticalPart(const std::string& n, int x, int y) : name(n), posX(x), posY(y) {}
    virtual ~OpticalPart() = default;
    virtual void burnIntoMap(std::vector<float>& indexMap, int width, int height) = 0;
    
    int getX() const { return posX; }
    int getY() const { return posY; }
    void setLocation(int x, int y) { posX = x; posY = y; }
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
    float rotationDegrees;

public:
    PrismPart(const std::string& n, int x, int y, int size, float rotation) 
        : OpticalPart(n, x, y), baseSize(size), rotationDegrees(rotation) {}
        
    void burnIntoMap(std::vector<float>& indexMap, int width, int height) override;
    
    int getSize() const { return baseSize; }
    void setSize(int s) { baseSize = s; }
    float getRotation() const { return rotationDegrees; }
    void setRotation(float r) { rotationDegrees = r; }
};

struct CustomLaserNode {
    int offsetX = 0; 
    int offsetY = 0; 
    float phaseValue = 0.0f;
    float phaseOffsetDeg = 0.0f;
    float frequencyOverride = 0.08f;
    float amplitudeScale = 1.0f;
    bool isActive = true;
};

class WaveEngine {
private:
    int width;
    int height;
    
    std::vector<float> gridCurrent;
    std::vector<float> gridPrevious;
    std::vector<float> gridIntensity;
    std::vector<float> refractiveIndexMap;
    std::vector<float> dampingMap;

    std::vector<std::unique_ptr<OpticalPart>> partsList;
    std::vector<CustomLaserNode> customSourceNodes;
    int selectedNodeIndex = -1; 

    int globalOriginX = 0;
    int globalOriginY = 0;

    float globalFrequency = 0.08f;
    float globalAmplitude = 0.8f;

    bool inputA_Active = true;
    bool inputB_Active = true;
    bool isPaused = false;
    bool showIntensityMode = false;

    bool saveSnapshotTriggered = false;

    int activeSceneMode = 0; 
    int activeColorPalette = 0; 

    void exportSurfaceToPNG(const std::string& filename, const std::vector<uint8_t>& rleBuffer);

public:
    WaveEngine(int w, int h);
    ~WaveEngine() = default;

    void addOpticalPart(std::unique_ptr<OpticalPart> part);
    void initSimulationSpace();
    
    // UPDATED: Independent allocation profiles per separate logical gating scene tracks
    void switchToAndGate();
    void switchToOrGate();
    void switchToNotGate();
    void switchToXorGate();
    
    bool loadSceneFromImage(const std::string& filePath);
    void switchToPrismScene();
    void clearGridFields();

    void generateNativeDoubleSlitPNG();
    void generateNativeBallLensPNG();
    void generateNativeFiberNetworkPNG();

    void stepSimulation();
    void updatePixels(uint32_t* pixelBuffer);

    void triggerSnapshotCapture() { saveSnapshotTriggered = true; }

    void setGlobalFrequency(float freq) { globalFrequency = freq; }
    void setGlobalAmplitude(float amp) { globalAmplitude = amp; }
    void setColorPalette(int paletteID) { activeColorPalette = paletteID; }
    void toggleInputs(bool bitA, bool bitB) { inputA_Active = bitA; inputB_Active = bitB; }
    
    void addIndependentSourceNode(int absoluteX, int absoluteY);
    void clearIndependentSourceNodes();
    std::vector<CustomLaserNode>& getCustomSourceNodes() { return customSourceNodes; }
    const std::vector<CustomLaserNode>& getCustomSourceNodes() const { return customSourceNodes; }
    
    int getGlobalOriginX() const { return globalOriginX; }
    int getGlobalOriginY() const { return globalOriginY; }
    void setGlobalOrigin(int gx, int gy) { globalOriginX = gx; globalOriginY = gy; }

    int getSelectedNodeIndex() const { return selectedNodeIndex; }
    void setSelectedNodeIndex(int index) { selectedNodeIndex = index; }

    int getActiveSceneMode() const { return activeSceneMode; }
    OpticalPart* getPrismPart() { return (!partsList.empty()) ? partsList[0].get() : nullptr; }

    float sampleAmplitude(int x, int y) const { 
        return (x >= 0 && x < width && y >= 0 && y < height) ? gridCurrent[y * width + x] : 0.0f; 
    }
    
    void setPauseState(bool pause) { isPaused = pause; }
    bool getPauseState() const { return isPaused; }

    void setIntensityMode(bool enable) { showIntensityMode = enable; }
    bool getIntensityMode() const { return showIntensityMode; }

    // --- Add these three lines for OpenEXR exporting ---
    float* getPrimaryFieldData() { return gridCurrent.empty() ? nullptr : gridCurrent.data(); }
    int getGridWidth() const { return width; }
    int getGridHeight() const { return height; }

};
