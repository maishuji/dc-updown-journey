// Copyright 2025 Quentin Cartier

#include "udjourney/scene/Scene.hpp"

#include <fstream>
#include <string>

// Include Dreamcast compatibility functions before nlohmann/json
#ifdef PLATFORM_DREAMCAST
#include "udjourney/dreamcast_json_compat.h"
#endif

#include <nlohmann/json.hpp>

#include <udj-core/Logger.hpp>

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
        
        // Load scene type (defaults to LEVEL for backward compatibility)
        if (scene_data.contains("scene_type")) {
            std::string type_str = scene_data["scene_type"].get<std::string>();
            if (type_str == "ui_screen") {
                m_scene_type = SceneType::UI_SCREEN;
            } else {
                m_scene_type = SceneType::LEVEL;
            }
        } else {
            m_scene_type = SceneType::LEVEL;
        }
        
        Logger::info("Scene type: %", m_scene_type == SceneType::LEVEL ? "LEVEL" : "UI_SCREEN");

        // Load player spawn (only for levels)
        if (m_scene_type == SceneType::LEVEL && scene_data.contains("player_spawn")) {
            const auto& spawn = scene_data["player_spawn"];
            m_player_spawn.tile_x = spawn.value("x", 0);
            m_player_spawn.tile_y = spawn.value("y", 0);
        }

        // Load platforms (only for levels)
        m_platforms.clear();
        if (m_scene_type == SceneType::LEVEL && scene_data.contains("platforms")) {
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

        // Load monster spawns (only for levels)
        m_monster_spawns.clear();
        if (m_scene_type == SceneType::LEVEL && scene_data.contains("monsters")) {
            for (const auto& monster_json : scene_data["monsters"]) {
                MonsterSpawnData monster;
                monster.tile_x = monster_json.value("x", 0);
                monster.tile_y = monster_json.value("y", 0);

                // New preset system - preferred approach
                monster.preset_name =
                    monster_json.value("preset_name", "goblin");

                // Legacy fields for backward compatibility
                monster.patrol_range =
                    monster_json.value("patrol_range", 100.0f);
                monster.chase_range = monster_json.value("chase_range", 200.0f);
                monster.attack_range =
                    monster_json.value("attack_range", 50.0f);
                monster.sprite_sheet =
                    monster_json.value("sprite_sheet", "char1-Sheet.png");
                m_monster_spawns.push_back(monster);
            }
        }

        // Load background layers
        m_background_layers.clear();
        if (scene_data.contains("backgrounds") &&
            scene_data["backgrounds"].contains("layers")) {
            Logger::info("Loading background layers...");
            for (const auto& layer_json : scene_data["backgrounds"]["layers"]) {
                BackgroundLayerData layer;
                layer.name = layer_json.value("name", "Background Layer");
                layer.texture_file = layer_json.value("texture_file", "");
                layer.parallax_factor =
                    layer_json.value("parallax_factor", 1.0f);
                layer.depth = layer_json.value("depth", 0);

                // Load background objects
                if (layer_json.contains("objects")) {
                    for (const auto& obj_json : layer_json["objects"]) {
                        BackgroundObjectData obj;
                        obj.sprite_name = obj_json.value("sprite_name", "");
                        obj.x = obj_json.value("x", 0.0f);
                        obj.y = obj_json.value("y", 0.0f);
                        obj.scale = obj_json.value("scale", 1.0f);
                        obj.rotation = obj_json.value("rotation", 0.0f);
                        obj.sprite_sheet = obj_json.value("sprite_sheet", "");
                        obj.tile_size = obj_json.value("tile_size", 128);
                        obj.tile_row = obj_json.value("tile_row", 0);
                        obj.tile_col = obj_json.value("tile_col", 0);
                        layer.objects.push_back(obj);
                    }
                }
                m_background_layers.push_back(layer);
                Logger::info("Loaded background layer: % with % objects",
                             layer.name,
                             layer.objects.size());
            }
        }

        // Load FUDs
        m_fuds.clear();
        if (scene_data.contains("fuds")) {
            Logger::info("Loading FUD elements...");
            for (const auto& fud_json : scene_data["fuds"]) {
                FUDData fud;
                fud.name = fud_json.value("name", "FUD");
                fud.type_id = fud_json.value("type_id", "unknown");

                // Parse anchor
                std::string anchor_str = fud_json.value("anchor", "TopLeft");
                if (anchor_str == "TopCenter")
                    fud.anchor = FUDAnchor::TopCenter;
                else if (anchor_str == "TopRight")
                    fud.anchor = FUDAnchor::TopRight;
                else if (anchor_str == "MiddleLeft")
                    fud.anchor = FUDAnchor::MiddleLeft;
                else if (anchor_str == "MiddleCenter")
                    fud.anchor = FUDAnchor::MiddleCenter;
                else if (anchor_str == "MiddleRight")
                    fud.anchor = FUDAnchor::MiddleRight;
                else if (anchor_str == "BottomLeft")
                    fud.anchor = FUDAnchor::BottomLeft;
                else if (anchor_str == "BottomCenter")
                    fud.anchor = FUDAnchor::BottomCenter;
                else if (anchor_str == "BottomRight")
                    fud.anchor = FUDAnchor::BottomRight;
                else
                    fud.anchor = FUDAnchor::TopLeft;

                // Load offset and size
                if (fud_json.contains("offset")) {
                    fud.offset_x = fud_json["offset"].value("x", 0.0f);
                    fud.offset_y = fud_json["offset"].value("y", 0.0f);
                }
                if (fud_json.contains("size")) {
                    fud.size_x = fud_json["size"].value("x", 100.0f);
                    fud.size_y = fud_json["size"].value("y", 30.0f);
                }

                fud.visible = fud_json.value("visible", true);

                // Load background image/sprite sheet configuration
                if (fud_json.contains("background_image")) {
                    fud.background_image =
                        fud_json["background_image"].get<std::string>();
                }
                if (fud_json.contains("background_sheet")) {
                    fud.background_sheet =
                        fud_json["background_sheet"].get<std::string>();
                    fud.background_tile_size =
                        fud_json.value("background_tile_size", 32);
                    fud.background_tile_row =
                        fud_json.value("background_tile_row", 0);
                    fud.background_tile_col =
                        fud_json.value("background_tile_col", 0);
                    fud.background_tile_width =
                        fud_json.value("background_tile_width", 1);
                    fud.background_tile_height =
                        fud_json.value("background_tile_height", 1);
                }

                // Load foreground image/sprite sheet configuration
                if (fud_json.contains("foreground_image")) {
                    fud.foreground_image =
                        fud_json["foreground_image"].get<std::string>();
                }
                if (fud_json.contains("foreground_sheet")) {
                    fud.foreground_sheet =
                        fud_json["foreground_sheet"].get<std::string>();
                    fud.foreground_tile_size =
                        fud_json.value("foreground_tile_size", 32);
                    fud.foreground_tile_row =
                        fud_json.value("foreground_tile_row", 0);
                    fud.foreground_tile_col =
                        fud_json.value("foreground_tile_col", 0);
                    fud.foreground_tile_width =
                        fud_json.value("foreground_tile_width", 1);
                    fud.foreground_tile_height =
                        fud_json.value("foreground_tile_height", 1);
                }

                if (fud_json.contains("image_scale")) {
                    fud.image_scale = fud_json["image_scale"].get<float>();
                }

                // Load properties as strings (simplified for game runtime)
                if (fud_json.contains("properties")) {
                    for (const auto& [key, value] :
                         fud_json["properties"].items()) {
                        // Store as plain string, not JSON-encoded
                        if (value.is_string()) {
                            fud.properties[key] = value.get<std::string>();
                        } else {
                            fud.properties[key] = value.dump();
                        }
                    }
                }

                m_fuds.push_back(fud);
                Logger::info("Loaded FUD: % (type: %)", fud.name, fud.type_id);
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
        
        // Save scene type
        scene_data["scene_type"] = (m_scene_type == SceneType::UI_SCREEN) ? "ui_screen" : "level";
        
        // Save player spawn (only for levels)
        if (m_scene_type == SceneType::LEVEL) {
            scene_data["player_spawn"]["x"] = m_player_spawn.tile_x;
            scene_data["player_spawn"]["y"] = m_player_spawn.tile_y;
        }

        // Save platforms (only for levels)
        if (m_scene_type == SceneType::LEVEL) {
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
        }  // end LEVEL-only platforms

        // Save monster spawns (only for levels)
        if (m_scene_type == SceneType::LEVEL) {
        json monsters_json = json::array();
        for (const auto& monster : m_monster_spawns) {
            json monster_json;
            monster_json["x"] = monster.tile_x;
            monster_json["y"] = monster.tile_y;
            monster_json["preset_name"] = monster.preset_name;

            // Include legacy fields for backward compatibility
            monster_json["patrol_range"] = monster.patrol_range;
            monster_json["chase_range"] = monster.chase_range;
            monster_json["attack_range"] = monster.attack_range;
            monster_json["sprite_sheet"] = monster.sprite_sheet;
            monsters_json.push_back(monster_json);
        }
        if (!monsters_json.empty()) {
            scene_data["monsters"] = monsters_json;
        }
        }  // end LEVEL-only monsters

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
