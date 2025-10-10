// Copyright 2025 Quentin Cartier

#include "udjourney/scene/Scene.hpp"

#include <fstream>
#include <string>

// Include Dreamcast compatibility functions before nlohmann/json
#ifdef PLATFORM_DREAMCAST
#include "udjourney/dreamcast_json_compat.h"
#endif

#include <nlohmann/json.hpp>

#include "udjourney/core/Logger.hpp"

using json = nlohmann::json;

namespace udjourney {
namespace scene {

Scene::Scene(const std::string& filename) { load_from_file(filename); }

Rectangle Scene::tile_to_world_rect(int tile_x, int tile_y, float width_tiles,
                                    float height_tiles) {
    return Rectangle{static_cast<float>(tile_x) * kTileSize,
                     static_cast<float>(tile_y) * kTileSize,
                     width_tiles * kTileSize,
                     height_tiles * kTileSize};
}

Vector2 Scene::tile_to_world_pos(int tile_x, int tile_y) {
    return Vector2{static_cast<float>(tile_x) * kTileSize,
                   static_cast<float>(tile_y) * kTileSize};
}

bool Scene::load_from_file(const std::string& filename) {
    Logger::info("Loading scene from file: %", filename);
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            Logger::error("Failed to open scene file: %", filename);
            return false;
        }

        json scene_data;
        file >> scene_data;

        // Load scene metadata
        m_name = scene_data.value("name", "Unnamed Level");

        // Load player spawn
        if (scene_data.contains("player_spawn")) {
            const auto& spawn = scene_data["player_spawn"];
            m_player_spawn.tile_x = spawn.value("x", 0);
            m_player_spawn.tile_y = spawn.value("y", 0);
        }

        // Load platforms
        m_platforms.clear();
        if (scene_data.contains("platforms")) {
            for (const auto& platform_json : scene_data["platforms"]) {
                PlatformData platform;

                platform.tile_x = platform_json.value("x", 0);
                platform.tile_y = platform_json.value("y", 0);
                platform.width_tiles = platform_json.value("width", 1.0f);
                platform.height_tiles = platform_json.value("height", 1.0f);

                // Load behavior
                std::string behavior_str =
                    platform_json.value("behavior", "static");
                if (behavior_str == "horizontal") {
                    platform.behavior_type = PlatformBehaviorType::Horizontal;
                } else if (behavior_str == "eight_turn") {
                    platform.behavior_type =
                        PlatformBehaviorType::EightTurnHorizontal;
                } else if (behavior_str == "oscillating_size") {
                    platform.behavior_type =
                        PlatformBehaviorType::OscillatingSize;
                } else {
                    platform.behavior_type = PlatformBehaviorType::Static;
                }

                // Load behavior parameters
                if (platform_json.contains("behavior_params")) {
                    for (const auto& [key, value] :
                         platform_json["behavior_params"].items()) {
                        platform.behavior_params[key] = value.get<float>();
                    }
                }

                // Load features
                Logger::info("Processing features for platform at tile (%, %)",
                             platform.tile_x,
                             platform.tile_y);
                if (platform_json.contains("features")) {
                    for (const auto& feature_str : platform_json["features"]) {
                        if (feature_str == "spikes") {
                            Logger::info(
                                "Loading spikes feature for platform at tile "
                                "(%, %)",
                                platform.tile_x,
                                platform.tile_y);
                            platform.features.push_back(
                                PlatformFeatureType::Spikes);
                        } else if (feature_str == "checkpoint") {
                            Logger::info(
                                "Loading checkpoint feature for platform at "
                                "tile (%, %)",
                                platform.tile_x,
                                platform.tile_y);
                            platform.features.push_back(
                                PlatformFeatureType::Checkpoint);
                        }
                    }
                }

                // Load feature parameters
                if (platform_json.contains("feature_params")) {
                    for (const auto& [key, value] :
                         platform_json["feature_params"].items()) {
                        platform.feature_params[key] = value.get<float>();
                    }
                }

                m_platforms.push_back(platform);
            }
        }

        Logger::info("Successfully loaded scene: %", filename);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Error loading scene %: %", filename, e.what());
        return false;
    }
}

bool Scene::save_to_file(const std::string& filename) const {
    try {
        json scene_data;

        scene_data["name"] = m_name;
        scene_data["player_spawn"]["x"] = m_player_spawn.tile_x;
        scene_data["player_spawn"]["y"] = m_player_spawn.tile_y;

        json platforms_json = json::array();
        for (const auto& platform : m_platforms) {
            json platform_json;
            platform_json["x"] = platform.tile_x;
            platform_json["y"] = platform.tile_y;
            platform_json["width"] = platform.width_tiles;
            platform_json["height"] = platform.height_tiles;

            // Save behavior type
            switch (platform.behavior_type) {
                case PlatformBehaviorType::Horizontal:
                    platform_json["behavior"] = "horizontal";
                    break;
                case PlatformBehaviorType::EightTurnHorizontal:
                    platform_json["behavior"] = "eight_turn";
                    break;
                case PlatformBehaviorType::OscillatingSize:
                    platform_json["behavior"] = "oscillating_size";
                    break;
                default:
                    platform_json["behavior"] = "static";
                    break;
            }

            // Save behavior parameters
            if (!platform.behavior_params.empty()) {
                platform_json["behavior_params"] = platform.behavior_params;
            }

            // Save features
            json features_json = json::array();
            for (auto feature : platform.features) {
                if (feature == PlatformFeatureType::Spikes) {
                    features_json.push_back("spikes");
                } else if (feature == PlatformFeatureType::Checkpoint) {
                    features_json.push_back("checkpoint");
                }
            }
            if (!features_json.empty()) {
                platform_json["features"] = features_json;
            }

            // Save feature parameters
            if (!platform.feature_params.empty()) {
                platform_json["feature_params"] = platform.feature_params;
            }

            platforms_json.push_back(platform_json);
        }
        scene_data["platforms"] = platforms_json;

        std::ofstream file(filename);
        if (!file.is_open()) {
            Logger::error("Failed to create scene file: %", filename);
            return false;
        }

        file << scene_data.dump(2);
        Logger::info("Successfully saved scene: %", filename);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Error saving scene %: %", filename, e.what());
        return false;
    }
}

}  // namespace scene
}  // namespace udjourney
