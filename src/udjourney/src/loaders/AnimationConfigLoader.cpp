// Copyright 2025 Quentin Cartier
// Include Dreamcast compatibility functions BEFORE nlohmann/json
#ifdef PLATFORM_DREAMCAST
#include "udjourney/dreamcast_json_compat.h"
#endif

#include "udjourney/loaders/AnimationConfigLoader.hpp"

#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>

#include "udjourney/core/Logger.hpp"
#include "udjourney/managers/TextureManager.hpp"
#include "udjourney/SpriteAnim.hpp"

using json = nlohmann::json;

namespace udjourney {
namespace loaders {

animation::AnimationPresetConfig AnimationConfigLoader::load_preset(
    const std::string& filename) {
    udjourney::Logger::info("Loading animation preset from: %", filename);

    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open animation config file: " +
                                 filename);
    }

    json j;
    try {
        file >> j;
    } catch (const json::parse_error& e) {
        throw std::runtime_error("Failed to parse animation config JSON: " +
                                 std::string(e.what()));
    }

    animation::AnimationPresetConfig config;

    // Load preset name
    if (j.contains("preset_name")) {
        config.preset_name = j["preset_name"].get<std::string>();
    }

    // Load animations array
    if (!j.contains("animations") || !j["animations"].is_array()) {
        throw std::runtime_error(
            "Animation config must contain 'animations' array");
    }

    for (const auto& anim_json : j["animations"]) {
        animation::AnimationStateConfig state_config;

        // Load animation name and state ID
        if (anim_json.contains("name")) {
            state_config.name = anim_json["name"].get<std::string>();
        }
        if (anim_json.contains("state_id")) {
            state_config.state_id = anim_json["state_id"].get<int>();
        }

        // Load sprite sheet configuration
        if (anim_json.contains("sprite_config")) {
            const auto& sprite_json = anim_json["sprite_config"];

            if (sprite_json.contains("filename")) {
                state_config.sprite_config.filename =
                    sprite_json["filename"].get<std::string>();
            }
            if (sprite_json.contains("sprite_width")) {
                state_config.sprite_config.sprite_width =
                    sprite_json["sprite_width"].get<int>();
            }
            if (sprite_json.contains("sprite_height")) {
                state_config.sprite_config.sprite_height =
                    sprite_json["sprite_height"].get<int>();
            }
            if (sprite_json.contains("frames") &&
                sprite_json["frames"].is_array()) {
                for (const auto& frame_json : sprite_json["frames"]) {
                    animation::FrameSpec frame;
                    if (frame_json.contains("row")) {
                        frame.row = frame_json["row"].get<int>();
                    }
                    if (frame_json.contains("col")) {
                        frame.col = frame_json["col"].get<int>();
                    }
                    state_config.sprite_config.frames.push_back(frame);
                }
            }
            if (sprite_json.contains("frame_duration")) {
                state_config.sprite_config.frame_duration =
                    sprite_json["frame_duration"].get<float>();
            }
            if (sprite_json.contains("loop")) {
                state_config.sprite_config.loop =
                    sprite_json["loop"].get<bool>();
            }
        }

        config.animations.push_back(state_config);
    }

    udjourney::Logger::info(
        "Successfully loaded animation preset '%' with % animations",
        config.preset_name,
        config.animations.size());

    return config;
}

AnimSpriteController AnimationConfigLoader::create_controller(
    const animation::AnimationPresetConfig& config) {
    AnimSpriteController controller;
    auto& texture_manager = TextureManager::get_instance();

    for (const auto& anim_config : config.animations) {
        const auto& sprite_cfg = anim_config.sprite_config;

        // Load texture
        Texture2D texture = texture_manager.get_texture(sprite_cfg.filename);

        // Frame count is simply the number of frames specified
        int frame_count = sprite_cfg.frames.size();

        // Get starting position from first frame (for now, assume linear
        // sequence)
        int start_row = 0;
        int start_col = 0;
        if (!sprite_cfg.frames.empty()) {
            start_row = sprite_cfg.frames[0].row;
            start_col = sprite_cfg.frames[0].col;
        }

        // Create SpriteAnim with starting position
        SpriteAnim sprite_anim(texture,
                               sprite_cfg.sprite_width,
                               sprite_cfg.sprite_height,
                               sprite_cfg.frame_duration,
                               frame_count,
                               sprite_cfg.loop,
                               start_row,
                               start_col);

        // Add animation to controller
        // If state_id is in PlayerState range (0-4), add as PlayerState enum
        // Otherwise, add as integer state (for MonsterState, etc.)
        if (anim_config.state_id >= 0 && anim_config.state_id <= 4) {
            // PlayerState range: IDLE=0, RUNNING=1, JUMPING=2, DASHING=3,
            // FALLING=4
            controller.add_animation(
                static_cast<PlayerState>(anim_config.state_id),
                anim_config.name,
                sprite_anim);
            udjourney::Logger::info(
                "Added PlayerState animation '%' (state %) with % frames from "
                "row %, col %",
                anim_config.name,
                anim_config.state_id,
                frame_count,
                start_row,
                start_col);
        } else {
            // Monster or other entity states
            controller.add_animation(
                anim_config.state_id, anim_config.name, sprite_anim);
            udjourney::Logger::info(
                "Added animation '%' (state %) with % frames from row %, col %",
                anim_config.name,
                anim_config.state_id,
                frame_count,
                start_row,
                start_col);
        }
    }

    return controller;
}

AnimSpriteController AnimationConfigLoader::load_and_create(
    const std::string& filename) {
    auto config = load_preset(filename);
    return create_controller(config);
}

}  // namespace loaders
}  // namespace udjourney
