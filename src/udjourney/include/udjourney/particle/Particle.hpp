// Copyright 2025 Quentin Cartier
#pragma once

#include "raylib/raylib.h"

namespace udjourney {

/**
 * @brief Individual particle instance
 */
struct Particle {
    Vector2 position{0.0f, 0.0f};
    Vector2 velocity{0.0f, 0.0f};
    Vector2 acceleration{0.0f, 0.0f};

    float lifetime = 1.0f;        // Total lifetime
    float age = 0.0f;             // Current age
    float rotation = 0.0f;        // Current rotation (degrees)
    float rotation_speed = 0.0f;  // Rotation per second

    Color start_color{255, 255, 255, 255};
    Color end_color{255, 255, 255, 0};
    float start_size = 4.0f;
    float end_size = 2.0f;

    Rectangle texture_rect{0, 0, 8, 8};  // Source rect in texture
    bool alive = true;

    /**
     * @brief Update particle state
     * @param delta Time step in seconds
     */
    void update(float delta);

    /**
     * @brief Get interpolated color based on age
     */
    [[nodiscard]] Color get_current_color() const;

    /**
     * @brief Get interpolated size based on age
     */
    [[nodiscard]] float get_current_size() const;

    /**
     * @brief Check if particle is dead
     */
    [[nodiscard]] bool is_dead() const { return !alive || age >= lifetime; }
};

}  // namespace udjourney
