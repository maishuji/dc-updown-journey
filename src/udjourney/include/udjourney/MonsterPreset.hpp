// Copyright 2025 Quentin Cartier
#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace udjourney {

struct StateTransition {
    std::string from_state;
    std::string to_state;
    std::string condition;  // e.g., "player_in_range", "animation_finished",
                            // "timer_expired"
    float condition_value = 0.0f;  // e.g., range distance, timer duration
};

struct MonsterStateConfig {
    std::string initial_state = "idle";
    std::vector<std::string> available_states;
    std::vector<StateTransition> transitions;
    std::unordered_map<std::string, float> state_durations;  // For timed states
};

struct MonsterStats {
    float max_health = 100.0f;
    float movement_speed = 50.0f;
    float damage = 25.0f;
    float knockback_force = 100.0f;
    float attack_cooldown = 2.0f;
    float jump_force = 0.0f;  // Added for jumping monsters
};

struct MonsterBehavior {
    float patrol_range = 128.0f;
    float chase_range = 200.0f;
    float attack_range = 50.0f;
    float patrol_speed_multiplier = 1.0f;
    float chase_speed_multiplier = 2.0f;

    // Movement capabilities
    bool can_jump = false;
    bool can_fly = false;
    bool can_climb = false;

    // AI timing
    float idle_duration = 2.0f;
    float jump_cooldown = 3.0f;
    float patrol_pause_duration = 1.0f;
};

struct MonsterPreset {
    std::string name;
    std::string display_name;
    std::string animation_preset_file;
    MonsterStats stats;
    MonsterBehavior behavior;
    MonsterStateConfig state_config;  // Flexible state machine

    // Visual properties
    float width = 64.0f;
    float height = 64.0f;
    std::string sprite_sheet;

    // Audio
    std::string attack_sound;
    std::string hurt_sound;
    std::string death_sound;
};

}  // namespace udjourney
