#include "engine/scene_loader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

bool loadSceneProfileFromJSON(WaveEngine& engine, int targetSceneID, const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        file.open("../" + configPath);
    }
    if (!file.is_open()) return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string contents = buffer.str();

    size_t scenePos = 0;
    while ((scenePos = contents.find("\"id\":", scenePos)) != std::string::npos) {
        size_t valStart = scenePos + 5;
        size_t valEnd = contents.find_first_of(",\n}", valStart);
        int currentID = std::stoi(contents.substr(valStart, valEnd - valStart));

        if (currentID == targetSceneID) {
            engine.clearIndependentSourceNodes();
            auto& nodes = engine.getCustomSourceNodes();

            size_t sourcesStart = contents.find("\"sources\":", scenePos);
            if (sourcesStart != std::string::npos) {
                size_t arrayEnd = contents.find("]", sourcesStart);
                size_t itemPos = sourcesStart;

                while ((itemPos = contents.find("{", itemPos)) != std::string::npos && itemPos < arrayEnd) {
                    auto getFloatAttr = [&](const std::string& attr) {
                        size_t attrPos = contents.find("\"" + attr + "\":", itemPos);
                        if (attrPos != std::string::npos) {
                            size_t s = attrPos + attr.length() + 3;
                            size_t e = contents.find_first_of(",}", s);
                            return std::stof(contents.substr(s, e - s));
                        }
                        return 0.0f;
                    };

                    int x = static_cast<int>(getFloatAttr("x"));
                    int y = static_cast<int>(getFloatAttr("y"));
                    float phase = getFloatAttr("phase");
                    float amp = getFloatAttr("amplitude");

                    engine.addIndependentSourceNode(x, y);
                    if (!nodes.empty()) {
                        nodes.back().phaseOffsetDeg = phase;
                        nodes.back().amplitudeScale = amp;
                    }
                    itemPos++;
                }
            }
            return true;
        }
        scenePos++;
    }
    return false;
}
