// Copyright 2025 Quentin Cartier
#pragma once
#include <raylib/raylib.h>

namespace udjourney {

class WorldBounds {
 public:
    WorldBounds() = default;
    WorldBounds(float left, float right, float top, float bottom);

    // Check if position is within bounds
    bool is_within_bounds(const Rectangle& rect) const;

    // Clamp position to bounds (for actors that should stay in bounds)
    Rectangle clamp_to_bounds(const Rectangle& rect) const;

    // Check collision with borders and return collision info
    struct BorderCollision {
        bool hit_left = false;
        bool hit_right = false;
        bool hit_top = false;
        bool hit_bottom = false;
        Rectangle corrected_rect;
    };

    BorderCollision check_border_collision(const Rectangle& rect) const;

    // Update bounds (useful for dynamic world sizes)
    void set_bounds(float left, float right, float top, float bottom);

    // Getters
    float get_left() const { return left_; }
    float get_right() const { return right_; }
    float get_top() const { return top_; }
    float get_bottom() const { return bottom_; }

    // Get bounds as rectangle for rendering debug info
    Rectangle get_bounds_rect() const;

 private:
    float left_ = 0.0f, right_ = 640.0f, top_ = 0.0f, bottom_ = 480.0f;
};

}  // namespace udjourney
