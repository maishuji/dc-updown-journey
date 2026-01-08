// Copyright 2025 Quentin Cartier
#include "udjourney-editor/strategies/level/ImportFromFileStrategy.hpp"

#include <fstream>
#include <iostream>
#include <string>

#include <nlohmann/json.hpp>
#include <udj-core/Logger.hpp>

#include "udjourney-editor/background/BackgroundManager.hpp"

ImportFromFileStrategy::ImportFromFileStrategy(const std::string& file_path,
                                               BackgroundManager* bg_manager) :
    file_path_(file_path), bg_manager_(bg_manager) {}

void ImportFromFileStrategy::create(Level& level, int tiles_x, int tiles_y) {
    import_success_ = false;
    error_message_.clear();

    std::ifstream in(file_path_);
    udj::core::Logger::info("Importing level from: %", file_path_);

    if (!in.is_open()) {
        error_message_ = "Failed to open file: " + file_path_;
        udj::core::Logger::error(error_message_);
        return;
    }

    try {
        nlohmann::json jlevel;
        in >> jlevel;
        in.close();

        // Clear existing data
        level.clear();

        // Load scene type (defaults to LEVEL)
        if (jlevel.contains("scene_type")) {
            std::string scene_type_str =
                jlevel["scene_type"].get<std::string>();
            level.scene_type = (scene_type_str == "ui_screen")
                                   ? SceneType::UI_SCREEN
                                   : SceneType::LEVEL;
            udj::core::Logger::info("Loaded scene type: %", scene_type_str);
        } else {
            level.scene_type = SceneType::LEVEL;
        }

        // Load scroll speed (defaults to 1.0)
        if (jlevel.contains("scroll_speed")) {
            level.scroll_speed = jlevel["scroll_speed"].get<float>();
            udj::core::Logger::info("Loaded scroll speed: %",
                                    level.scroll_speed);
        } else {
            level.scroll_speed = 1.0f;  // Default
        }

        // Load physics config
        level.physics_config.gravity = jlevel.value("gravity", 0.5f);
        level.physics_config.terminal_velocity =
            jlevel.value("terminal_velocity", 10.0f);

        // Get level dimensions from JSON if available, otherwise use provided
        int level_rows = tiles_y;
        int level_cols = tiles_x;

        if (jlevel.contains("rows") && jlevel.contains("cols")) {
            level_rows = jlevel["rows"].get<int>();
            level_cols = jlevel["cols"].get<int>();
        }

        level.resize(level_rows, level_cols);

        // Fill with empty tiles
        Cell empty_cell;
        for (int i = 0; i < level_cols * level_rows; ++i) {
            level.push_back(empty_cell);
        }

        // Import player spawn position (only for levels)
        if (level.scene_type == SceneType::LEVEL &&
            jlevel.contains("player_spawn") &&
            jlevel["player_spawn"].is_object()) {
            const auto& spawn = jlevel["player_spawn"];
            if (spawn.contains("x") && spawn.contains("y")) {
                level.player_spawn_x = spawn["x"].get<int>();
                level.player_spawn_y = spawn["y"].get<int>();
            }
        }

        // Import platforms (only for levels)
        if (level.scene_type == SceneType::LEVEL &&
            jlevel.contains("platforms") && jlevel["platforms"].is_array()) {
            for (const auto& jplatform : jlevel["platforms"]) {
                // Skip invalid platform entries
                if (!jplatform.is_object() || !jplatform.contains("x") ||
                    !jplatform.contains("y") || !jplatform.contains("width") ||
                    !jplatform.contains("height") ||
                    !jplatform.contains("behavior")) {
                    continue;
                }

                EditorPlatform platform;
                platform.tile_x = jplatform["x"].get<float>();
                platform.tile_y = jplatform["y"].get<float>();
                platform.width_tiles = jplatform["width"].get<float>();
                platform.height_tiles = jplatform["height"].get<float>();

                // Parse behavior type
                std::string behavior = jplatform["behavior"].get<std::string>();
                if (behavior == "static") {
                    platform.behavior_type = PlatformBehaviorType::Static;
                } else if (behavior == "horizontal") {
                    platform.behavior_type = PlatformBehaviorType::Horizontal;
                } else if (behavior == "eight_turn") {
                    platform.behavior_type =
                        PlatformBehaviorType::EightTurnHorizontal;
                } else if (behavior == "oscillating_size") {
                    platform.behavior_type =
                        PlatformBehaviorType::OscillatingSize;
                }

                // Parse features
                platform.features.clear();
                if (jplatform.contains("features")) {
                    for (const auto& feature_str : jplatform["features"]) {
                        std::string feature = feature_str.get<std::string>();
                        if (feature == "spikes") {
                            platform.features.push_back(
                                PlatformFeatureType::Spikes);
                        } else if (feature == "checkpoint") {
                            platform.features.push_back(
                                PlatformFeatureType::Checkpoint);
                        }
                    }
                }

                // Load behavior parameters
                platform.behavior_params.clear();
                if (jplatform.contains("behavior_params") &&
                    jplatform["behavior_params"].is_object()) {
                    for (auto& [key, value] :
                         jplatform["behavior_params"].items()) {
                        if (value.is_number()) {
                            platform.behavior_params[key] = value.get<float>();
                        }
                    }
                }

                // Optional platform texture
                platform.texture_file = jplatform.value(
                    "texture", jplatform.value("texture_file", ""));
                platform.texture_tiled =
                    jplatform.value("texture_tiled", false);

                // Set platform color based on behavior and features
                PlatformFeatureType primary_feature =
                    platform.features.empty() ? PlatformFeatureType::None
                                              : platform.features[0];

                // Simple color assignment based on behavior type
                switch (platform.behavior_type) {
                    case PlatformBehaviorType::Static:
                        platform.color = IM_COL32(0, 0, 255, 255);  // Blue
                        break;
                    case PlatformBehaviorType::Horizontal:
                        platform.color = IM_COL32(255, 128, 0, 255);  // Orange
                        break;
                    case PlatformBehaviorType::EightTurnHorizontal:
                        platform.color = IM_COL32(128, 0, 255, 255);  // Purple
                        break;
                    case PlatformBehaviorType::OscillatingSize:
                        platform.color =
                            IM_COL32(0, 255, 128, 255);  // Light Green
                        break;
                }

                // Override with feature colors if present
                if (primary_feature == PlatformFeatureType::Spikes) {
                    platform.color =
                        IM_COL32(255, 0, 0, 255);  // Red for spikes
                } else if (primary_feature == PlatformFeatureType::Checkpoint) {
                    platform.color =
                        IM_COL32(0, 255, 0, 255);  // Green for checkpoint
                }

                level.platforms.push_back(platform);
            }
        }

        // Import monsters (only for levels)
        if (level.scene_type == SceneType::LEVEL &&
            jlevel.contains("monsters")) {
            for (const auto& jmonster : jlevel["monsters"]) {
                EditorMonster monster;
                monster.tile_x = jmonster["x"].get<int>();
                monster.tile_y = jmonster["y"].get<int>();
                monster.preset_name =
                    jmonster["preset_name"].get<std::string>();

                // Load health and speed overrides if present
                if (jmonster.contains("health")) {
                    monster.health_override = jmonster["health"].get<int>();
                }
                if (jmonster.contains("speed")) {
                    monster.speed_override = jmonster["speed"].get<int>();
                }

                // Set color based on preset
                if (monster.preset_name == "goblin") {
                    monster.color = IM_COL32(255, 0, 0, 255);  // Red
                } else if (monster.preset_name == "spider") {
                    monster.color = IM_COL32(128, 0, 128, 255);  // Purple
                } else {
                    monster.color = IM_COL32(255, 0, 0, 255);  // Default red
                }

                level.monsters.push_back(monster);
            }
        }

        // Import background data if BackgroundManager is provided
        if (bg_manager_ && jlevel.contains("backgrounds")) {
            bg_manager_->clear();
            bg_manager_->from_json(jlevel["backgrounds"]);
        }

        // Import FUDs
        if (jlevel.contains("huds") && jlevel["huds"].is_array()) {
            for (const auto& jfud : jlevel["huds"]) {
                try {
                    HUDElement hud;
                    from_json(jfud, hud);  // Explicit call to from_json
                    level.huds.push_back(hud);
                } catch (const std::exception& e) {
                    udj::core::Logger::error("Failed to load FUD: %", e.what());
                }
            }
        }

        import_success_ = true;
        udj::core::Logger::info("Successfully imported level from: %",
                                file_path_);
    } catch (const std::exception& e) {
        error_message_ = std::string("Error importing level JSON: ") + e.what();
        udj::core::Logger::error("%", error_message_);
        import_success_ = false;
    }
}
