// Copyright 2025 Quentin Cartier
#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace udjourney {
namespace animation {

/**
 * Single frame specification within a sprite sheet
 */
struct FrameSpec {
    int row = 0;  // Row index in the sprite sheet
    int col = 0;  // Column index in the sprite sheet
};

/**
 * Configuration for a single sprite sheet used in an animation
 */
struct SpriteSheetConfig {
    std::string filename;    // Path to sprite sheet image
    int sprite_width = 64;   // Width of each sprite in pixels
    int sprite_height = 64;  // Height of each sprite in pixels
    std::vector<FrameSpec>
        frames;  // Specific frames to use for this animation (in order)
    float frame_duration = 0.1f;  // Duration per frame in seconds
    bool loop = true;             // Should animation loop
};

/**
 * Configuration for a single animation state
 */
struct AnimationStateConfig {
    std::string name;  // Name of the animation state (e.g., "idle", "running")
    int state_id =
        0;  // Numeric ID for the state (maps to PlayerState enum value)
    SpriteSheetConfig sprite_config;
};

/**
 * Complete animation configuration for an actor (player, monster, etc.)
 */
struct AnimationPresetConfig {
    std::string
        preset_name;  // Name of this preset (e.g., "player", "monster_goblin")
    std::vector<AnimationStateConfig> animations;
};

}  // namespace animation
}  // namespace udjourney
