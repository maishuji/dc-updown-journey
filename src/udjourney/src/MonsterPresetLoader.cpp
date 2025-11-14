// Copyright 2025 Quentin Cartier
#include "udjourney/loaders/MonsterPresetLoader.hpp"

#include <fstream>
#include <stdexcept>
#include <string>
#include <iostream>
#include <set>

#include "udjourney/CoreUtils.hpp"

using json = nlohmann::json;

namespace udjourney {

std::unique_ptr<MonsterPreset> MonsterPresetLoader::load_preset(
    const std::string& preset_file) {
    std::string full_path =
        udjourney::coreutils::get_assets_path("monsters/" + preset_file);

    std::cout << "[INFO] Loading monster preset from: " << full_path
              << std::endl;

    std::ifstream file(full_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open monster preset file: " +
                                 full_path);
    }

    json j;
    file >> j;

    auto preset = std::make_unique<MonsterPreset>();

    // Basic info
    preset->name = j.at("name").get<std::string>();
    preset->display_name = j.value("display_name", preset->name);
    preset->animation_preset_file = j.at("animation_preset").get<std::string>();

    // Visual properties
    if (j.contains("visual")) {
        const auto& visual = j["visual"];
        preset->width = visual.value("width", 64.0f);
        preset->height = visual.value("height", 64.0f);
        preset->sprite_sheet = visual.value("sprite_sheet", "char1-Sheet.png");
    }

    // Stats
    if (j.contains("stats")) {
        preset->stats = parse_stats(j["stats"]);
    }

    // Behavior
    if (j.contains("behavior")) {
        preset->behavior = parse_behavior(j["behavior"]);
    }

    // State configuration
    if (j.contains("state_config")) {
        preset->state_config = parse_state_config(j["state_config"]);
    }

    // Audio (optional)
    if (j.contains("audio")) {
        const auto& audio = j["audio"];
        preset->attack_sound = audio.value("attack", "");
        preset->hurt_sound = audio.value("hurt", "");
        preset->death_sound = audio.value("death", "");
    }

    std::cout << "[INFO] Successfully loaded monster preset '" << preset->name
              << "'" << std::endl;

    return preset;
}

MonsterStats MonsterPresetLoader::parse_stats(const nlohmann::json& json) {
    MonsterStats stats;
    stats.max_health = json.value("max_health", 100.0f);
    stats.movement_speed = json.value("movement_speed", 50.0f);
    stats.damage = json.value("damage", 25.0f);
    stats.knockback_force = json.value("knockback_force", 100.0f);
    stats.attack_cooldown = json.value("attack_cooldown", 2.0f);
    stats.jump_force = json.value("jump_force", 0.0f);
    return stats;
}

MonsterBehavior MonsterPresetLoader::parse_behavior(
    const nlohmann::json& json) {
    MonsterBehavior behavior;
    behavior.patrol_range = json.value("patrol_range", 128.0f);
    behavior.chase_range = json.value("chase_range", 200.0f);
    behavior.attack_range = json.value("attack_range", 50.0f);
    behavior.patrol_speed_multiplier =
        json.value("patrol_speed_multiplier", 1.0f);
    behavior.chase_speed_multiplier =
        json.value("chase_speed_multiplier", 2.0f);
    behavior.can_jump = json.value("can_jump", false);
    behavior.can_fly = json.value("can_fly", false);
    behavior.can_climb = json.value("can_climb", false);
    behavior.idle_duration = json.value("idle_duration", 2.0f);
    behavior.jump_cooldown = json.value("jump_cooldown", 3.0f);
    behavior.patrol_pause_duration = json.value("patrol_pause_duration", 1.0f);
    return behavior;
}

MonsterStateConfig MonsterPresetLoader::parse_state_config(
    const nlohmann::json& json) {
    MonsterStateConfig config;

    config.initial_state = json.value("initial_state", "idle");

    if (json.contains("available_states")) {
        for (const auto& state : json["available_states"]) {
            config.available_states.push_back(state.get<std::string>());
        }
    }

    if (json.contains("transitions")) {
        for (const auto& transition : json["transitions"]) {
            StateTransition trans;
            trans.from_state = transition.at("from_state").get<std::string>();
            trans.to_state = transition.at("to_state").get<std::string>();
            trans.condition = transition.at("condition").get<std::string>();
            trans.condition_value = transition.value("condition_value", 0.0f);
            config.transitions.push_back(trans);
        }
    }

    if (json.contains("state_durations")) {
        for (const auto& [state, duration] : json["state_durations"].items()) {
            config.state_durations[state] = duration.get<float>();
        }
    }

    // Validate state configuration
    std::set<std::string> valid_states = {"idle", "patrol", "chase", "attack", "hurt", "death"};
    
    // Check if initial state is valid
    if (valid_states.find(config.initial_state) == valid_states.end()) {
        std::cerr << "WARNING: Initial state '" << config.initial_state 
                  << "' is not a recognized state. Valid states are: ";
        for (const auto& state : valid_states) {
            std::cerr << state << " ";
        }
        std::cerr << std::endl;
    }
    
    // Check if all available states are valid
    for (const auto& state : config.available_states) {
        if (valid_states.find(state) == valid_states.end()) {
            std::cerr << "WARNING: Available state '" << state 
                      << "' is not a recognized state. Valid states are: ";
            for (const auto& valid_state : valid_states) {
                std::cerr << valid_state << " ";
            }
            std::cerr << std::endl;
        }
    }

    return config;
}

}  // namespace udjourney
