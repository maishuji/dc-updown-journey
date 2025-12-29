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

namespace {
SceneType get_scene_type_(json& scene_data) {
    if (scene_data.contains("scene_type")) {
        std::string type_str = scene_data["scene_type"].get<std::string>();
        if (type_str == "ui_screen") {
            return SceneType::UiScreen;
        } else {
            return SceneType::Level;
        }
    } else {
        return SceneType::Level;
    }
}

void load_level_platforms_(const json& scene_data,
                           std::vector<PlatformData>& platforms) {
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
                platform.behavior_type = PlatformBehaviorType::OscillatingSize;
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

            platforms.push_back(platform);
        }
    }
}

void load_level_monsters_(
    json& scene_data, std::vector<scene::MonsterSpawnData>& monster_spawns) {
    if (scene_data.contains("monsters")) {
        for (const auto& monster_json : scene_data["monsters"]) {
            MonsterSpawnData monster;
            monster.tile_x = monster_json.value("x", 0);
            monster.tile_y = monster_json.value("y", 0);

            // New preset system - preferred approach
            monster.preset_name = monster_json.value("preset_name", "goblin");

            // Legacy fields for backward compatibility
            monster.patrol_range = monster_json.value("patrol_range", 100.0f);
            monster.chase_range = monster_json.value("chase_range", 200.0f);
            monster.attack_range = monster_json.value("attack_range", 50.0f);
            monster.sprite_sheet =
                monster_json.value("sprite_sheet", "char1-Sheet.png");
            monster_spawns.push_back(monster);
        }
    }
}

void load_background_(
    const json& scene_data,
    std::vector<scene::BackgroundLayerData>& background_layers) {
    if (scene_data.contains("backgrounds") &&
        scene_data["backgrounds"].contains("layers")) {
        Logger::info("Loading background layers...");
        for (const auto& layer_json : scene_data["backgrounds"]["layers"]) {
            BackgroundLayerData layer;
            layer.name = layer_json.value("name", "Background Layer");
            layer.texture_file = layer_json.value("texture_file", "");
            layer.parallax_factor = layer_json.value("parallax_factor", 1.0f);
            layer.depth = layer_json.value("depth", 0);

            // Load auto-scroll properties
            layer.auto_scroll_enabled =
                layer_json.value("auto_scroll_enabled", true);
            layer.scroll_speed_x = layer_json.value("scroll_speed_x", 0.0f);
            layer.scroll_speed_y = layer_json.value("scroll_speed_y", 0.0f);
            layer.repeat = layer_json.value("repeat", false);

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
            background_layers.push_back(layer);
            Logger::info("Loaded background layer: % with % objects",
                         layer.name,
                         layer.objects.size());
        }
    }
}

void load_fuds_(const json& scene_data, std::vector<scene::HUDData>& huds) {
    if (scene_data.contains("huds")) {
        Logger::info("Loading HUD elements...");
        for (const auto& fud_json : scene_data["huds"]) {
            HUDData hud;
            hud.name = fud_json.value("name", "HUD");
            hud.type_id = fud_json.value("type_id", "unknown");

            // Parse anchor
            std::string anchor_str = fud_json.value("anchor", "TopLeft");
            if (anchor_str == "TopCenter")
                hud.anchor = HUDAnchor::TopCenter;
            else if (anchor_str == "TopRight")
                hud.anchor = HUDAnchor::TopRight;
            else if (anchor_str == "MiddleLeft")
                hud.anchor = HUDAnchor::MiddleLeft;
            else if (anchor_str == "MiddleCenter")
                hud.anchor = HUDAnchor::MiddleCenter;
            else if (anchor_str == "MiddleRight")
                hud.anchor = HUDAnchor::MiddleRight;
            else if (anchor_str == "BottomLeft")
                hud.anchor = HUDAnchor::BottomLeft;
            else if (anchor_str == "BottomCenter")
                hud.anchor = HUDAnchor::BottomCenter;
            else if (anchor_str == "BottomRight")
                hud.anchor = HUDAnchor::BottomRight;
            else
                hud.anchor = HUDAnchor::TopLeft;

            // Load offset and size
            if (fud_json.contains("offset")) {
                hud.offset_x = fud_json["offset"].value("x", 0.0f);
                hud.offset_y = fud_json["offset"].value("y", 0.0f);
            }
            if (fud_json.contains("size")) {
                hud.size_x = fud_json["size"].value("x", 100.0f);
                hud.size_y = fud_json["size"].value("y", 30.0f);
            }

            hud.visible = fud_json.value("visible", true);

            // Load background image/sprite sheet configuration
            if (fud_json.contains("background_sheet")) {
                hud.background_sheet =
                    fud_json["background_sheet"].get<std::string>();
                hud.background_tile_size =
                    fud_json.value("background_tile_size", 32);
                hud.background_tile_row =
                    fud_json.value("background_tile_row", 0);
                hud.background_tile_col =
                    fud_json.value("background_tile_col", 0);
                hud.background_tile_width =
                    fud_json.value("background_tile_width", 1);
                hud.background_tile_height =
                    fud_json.value("background_tile_height", 1);
            }

            // Load foreground image/sprite sheet configuration
            if (fud_json.contains("foreground_sheet")) {
                hud.foreground_sheet =
                    fud_json["foreground_sheet"].get<std::string>();
                hud.foreground_tile_size =
                    fud_json.value("foreground_tile_size", 32);
                hud.foreground_tile_row =
                    fud_json.value("foreground_tile_row", 0);
                hud.foreground_tile_col =
                    fud_json.value("foreground_tile_col", 0);
                hud.foreground_tile_width =
                    fud_json.value("foreground_tile_width", 1);
                hud.foreground_tile_height =
                    fud_json.value("foreground_tile_height", 1);
            }

            if (fud_json.contains("image_scale")) {
                hud.image_scale = fud_json["image_scale"].get<float>();
            }

            // Load properties as strings (simplified for game runtime)
            if (fud_json.contains("properties")) {
                for (const auto& [key, value] :
                     fud_json["properties"].items()) {
                    // Store as plain string, not JSON-encoded
                    if (value.is_string()) {
                        hud.properties[key] = value.get<std::string>();
                    } else {
                        hud.properties[key] = value.dump();
                    }
                }
            }

            huds.push_back(hud);
            Logger::info("Loaded FUD: % (type: %)", hud.name, hud.type_id);
        }
    }
}

}  // namespace

Scene::Scene(const std::string& filename) { load_from_file(filename); }

Rectangle Scene::tile_to_world_rect(int tile_x, int tile_y, float width_tiles,
                                    float height_tiles) {
    // Platform is centered on the tile position
    // tile_x, tile_y represent the CENTER of the platform
    float center_x = static_cast<float>(tile_x) * kTileSize + kTileSize / 2;
    float center_y = static_cast<float>(tile_y) * kTileSize + kTileSize / 2;
    float width = width_tiles * kTileSize;
    float height = height_tiles * kTileSize;

    return Rectangle{
        center_x - width / 2, center_y - height / 2, width, height};
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
        m_scroll_speed = scene_data.value("scroll_speed", 1.0f);

        // Load scene type (defaults to LEVEL for backward compatibility)

        m_scene_type = get_scene_type_(scene_data);

        Logger::info("Scene type: %",
                     m_scene_type == SceneType::Level ? "LEVEL" : "UI_SCREEN");

        // Load player spawn (only for levels)
        if (m_scene_type == SceneType::Level &&
            scene_data.contains("player_spawn")) {
            const auto& spawn = scene_data["player_spawn"];
            m_player_spawn.tile_x = spawn.value("x", 0);
            m_player_spawn.tile_y = spawn.value("y", 0);
        }

        // Load platforms and monsters only for levels)
        m_platforms.clear();
        m_monster_spawns.clear();

        if (m_scene_type == SceneType::Level) {
            load_level_platforms_(scene_data, m_platforms);
            load_level_monsters_(scene_data, m_monster_spawns);
        }

        // Load background layers
        m_background_layers.clear();
        load_background_(scene_data, m_background_layers);

        // Load FUDs
        m_huds.clear();
        load_fuds_(scene_data, m_huds);

        // Load game menu (only for levels)
        m_game_menu = GameMenuData{};  // Reset
        if (m_scene_type == SceneType::Level &&
            scene_data.contains("game_menu")) {
            const auto& menu_json = scene_data["game_menu"];

            m_game_menu.title = menu_json.value("title", "GAME MENU");

            if (menu_json.contains("rect")) {
                const auto& rect = menu_json["rect"];
                m_game_menu.rect = {rect.value("x", 0.0f),
                                    rect.value("y", 0.0f),
                                    rect.value("width", 640.0f),
                                    rect.value("height", 480.0f)};
            }

            if (menu_json.contains("items")) {
                for (const auto& item_json : menu_json["items"]) {
                    MenuItemData item;
                    item.label = item_json.value("label", "");
                    item.action = item_json.value("action", "");

                    if (item_json.contains("params")) {
                        for (const auto& param : item_json["params"]) {
                            item.params.push_back(param.get<std::string>());
                        }
                    }

                    m_game_menu.items.push_back(item);
                }
            }

            Logger::info("Loaded game menu with % items",
                         m_game_menu.items.size());
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
        scene_data["scroll_speed"] = m_scroll_speed;

        // Save scene type
        scene_data["scene_type"] =
            (m_scene_type == SceneType::UiScreen) ? "ui_screen" : "level";

        // Save player spawn (only for levels)
        if (m_scene_type == SceneType::Level) {
            scene_data["player_spawn"]["x"] = m_player_spawn.tile_x;
            scene_data["player_spawn"]["y"] = m_player_spawn.tile_y;
        }

        // Save platforms (only for levels)
        if (m_scene_type == SceneType::Level) {
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
        if (m_scene_type == SceneType::Level) {
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
