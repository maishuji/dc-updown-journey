// Copyright 2025 Quentin Cartier
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace udjourney {
namespace animation {

/**
 * Collision bounds relative to sprite position
 */
struct CollisionBounds {
    float offset_x = 0.0f;  // X offset from sprite position
    float offset_y = 0.0f;  // Y offset from sprite position
    float width = 0.0f;     // Width of collision box
    float height = 0.0f;    // Height of collision box

    // Check if bounds are defined (width and height > 0)
    bool is_valid() const { return width > 0.0f && height > 0.0f; }
};

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
    std::string filename;     // Path to sprite sheet image
    int sprite_width = 64;    // Width of each sprite in pixels
    int sprite_height = 64;   // Height of each sprite in pixels
    std::vector<FrameSpec> frames;  // Frames to use (in order)
    float frame_duration = 0.1f;    // Duration per frame in seconds
    bool loop = true;               // Should animation loop
};

/**
 * Configuration for a single animation state
 */
struct AnimationStateConfig {
    std::string name;  // Name of the animation state (e.g., "idle")
    int state_id = 0;  // Numeric ID (maps to runtime state)
    CollisionBounds collision_bounds;  // Optional collision bounds
    SpriteSheetConfig sprite_config;
};

/**
 * Complete animation configuration for an actor (player, monster, etc.)
 */
struct AnimationPresetConfig {
    std::string preset_name;
    std::vector<AnimationStateConfig> animations;
};

}  // namespace animation
}  // namespace udjourney
