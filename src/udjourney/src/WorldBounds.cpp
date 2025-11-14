// Copyright 2025 Quentin Cartier
#include "udjourney/WorldBounds.hpp"

#include <algorithm>

namespace udjourney {

WorldBounds::WorldBounds(float left, float right, float top, float bottom) :
    left_(left), right_(right), top_(top), bottom_(bottom) {}

bool WorldBounds::is_within_bounds(const Rectangle& rect) const {
    return rect.x >= left_ && rect.x + rect.width <= right_ && rect.y >= top_ &&
           rect.y + rect.height <= bottom_;
}

Rectangle WorldBounds::clamp_to_bounds(const Rectangle& rect) const {
    Rectangle clamped = rect;

    // Clamp X position
    clamped.x = std::max(left_, std::min(right_ - rect.width, rect.x));

    // Clamp Y position
    clamped.y = std::max(top_, std::min(bottom_ - rect.height, rect.y));

    return clamped;
}

WorldBounds::BorderCollision WorldBounds::check_border_collision(
    const Rectangle& rect) const {
    BorderCollision collision;
    collision.corrected_rect = rect;

    // Check left border
    if (rect.x < left_) {
        collision.hit_left = true;
        collision.corrected_rect.x = left_;
    }

    // Check right border
    if (rect.x + rect.width > right_) {
        collision.hit_right = true;
        collision.corrected_rect.x = right_ - rect.width;
    }

    // Check top border
    if (rect.y < top_) {
        collision.hit_top = true;
        collision.corrected_rect.y = top_;
    }

    // Check bottom border
    if (rect.y + rect.height > bottom_) {
        collision.hit_bottom = true;
        collision.corrected_rect.y = bottom_ - rect.height;
    }

    return collision;
}

void WorldBounds::set_bounds(float left, float right, float top, float bottom) {
    left_ = left;
    right_ = right;
    top_ = top;
    bottom_ = bottom;
}

Rectangle WorldBounds::get_bounds_rect() const {
    return Rectangle{left_, top_, right_ - left_, bottom_ - top_};
}

}  // namespace udjourney
