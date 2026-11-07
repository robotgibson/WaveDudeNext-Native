#pragma once
#include "engine/WaveEngine.hpp"
#include <string>

// Restored to its original layout; handles source nodes setup from JSON
bool loadSceneProfileFromJSON(WaveEngine& engine, int targetSceneID, const std::string& configPath = "config/scene_profiles.json");
