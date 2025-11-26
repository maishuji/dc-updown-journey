// Copyright 2025 Quentin Cartier
#include "udjourney-editor/strategies/level/StaircaseLevelStrategy.hpp"

#include <algorithm>

StaircaseLevelStrategy::StaircaseLevelStrategy(int step_width,
                                               int step_height) :
    step_width_(step_width), step_height_(step_height) {}

void StaircaseLevelStrategy::create(Level& level, int tiles_x, int tiles_y) {
    level.clear();
    level.resize(tiles_y, tiles_x);

    // Fill with empty tiles
    Cell empty_cell;
    for (int i = 0; i < tiles_x * tiles_y; ++i) {
        level.push_back(empty_cell);
    }

    // Start at top for descending staircase
    int current_x = 0;
    int current_y = 2;
    bool going_down = true;  // Start going down

    // Set player spawn at the first platform position
    level.player_spawn_x = 2;
    level.player_spawn_y = current_y;

    // Create zigzag descending staircase platforms
    bool moving_right = true;                // Start moving right
    const int margin = 2;                    // Margin from edges before turning
    int max_iterations = tiles_x * tiles_y;  // Safety limit
    int iteration = 0;

    while (iteration < max_iterations) {
        iteration++;

        // Check bounds before creating platform
        if (current_x < 0 || current_x > tiles_x || current_y < margin ||
            current_y > tiles_y - 1) {
            break;
        }

        // Create platform
        EditorPlatform platform;
        platform.tile_x = current_x;
        platform.tile_y = current_y;
        platform.width_tiles = static_cast<float>(step_width_);
        platform.height_tiles = 1.0f;
        platform.behavior_type = PlatformBehaviorType::Static;
        platform.color = IM_COL32(0, 0, 255, 255);  // Blue for regular

        level.platforms.push_back(platform);

        // Check if we need to turn before moving

        bool should_turn = false;

        if (moving_right) {
            // Check if next step would go past right edge
            if (current_x + step_width_ >= tiles_x - margin) {
                should_turn = true;
            }
        } else {
            // Check if next step would go past left edge
            if (current_x - step_width_ < margin) {
                should_turn = true;
            }
        }

        if (should_turn) {
            // Mark the current platform as checkpoint before turning
            auto& last_platform = level.platforms.back();
            last_platform.color = IM_COL32(0, 255, 0, 255);  // Green
            last_platform.features.push_back(PlatformFeatureType::Checkpoint);

            // Change horizontal direction only
            moving_right = !moving_right;

            // Position at the opposite edge for next row
            if (moving_right) {
                current_x = 0;
            } else {
                current_x = tiles_x - 1;
            }
            // Move down vertically
            current_y += step_height_;
        } else {
            // Move horizontally based on current direction
            if (moving_right) {
                current_x += step_width_;
            } else {
                current_x -= step_width_;
            }
        }

        // Always move down vertically
        current_y += step_height_;
    }

    // Ensure the last platform is marked as checkpoint
    if (!level.platforms.empty()) {
        auto& last_platform = level.platforms.back();
        if (std::find(last_platform.features.begin(),
                      last_platform.features.end(),
                      PlatformFeatureType::Checkpoint) ==
            last_platform.features.end()) {
            last_platform.features.push_back(PlatformFeatureType::Checkpoint);
            last_platform.color = IM_COL32(0, 255, 0, 255);
        }
    }
}
