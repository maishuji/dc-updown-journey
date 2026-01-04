// Copyright 2025 Quentin Cartier
#include "udjourney/particle/Particle.hpp"
#include <algorithm>

namespace udjourney {

void Particle::update(float delta) {
    if (!alive) return;

    age += delta;

    // Update physics
    velocity.x += acceleration.x * delta;
    velocity.y += acceleration.y * delta;
    position.x += velocity.x * delta;
    position.y += velocity.y * delta;

    // Update rotation
    rotation += rotation_speed * delta;

    // Check lifetime
    if (age >= lifetime) {
        alive = false;
    }
}

Color Particle::get_current_color() const {
    if (age >= lifetime) return end_color;

    float t = age / lifetime;

    // Linear interpolation between start and end colors
    Color result;
    result.r = static_cast<unsigned char>(start_color.r +
                                          (end_color.r - start_color.r) * t);
    result.g = static_cast<unsigned char>(start_color.g +
                                          (end_color.g - start_color.g) * t);
    result.b = static_cast<unsigned char>(start_color.b +
                                          (end_color.b - start_color.b) * t);
    result.a = static_cast<unsigned char>(start_color.a +
                                          (end_color.a - start_color.a) * t);

    return result;
}

float Particle::get_current_size() const {
    if (age >= lifetime) return end_size;

    float t = age / lifetime;
    return start_size + (end_size - start_size) * t;
}

}  // namespace udjourney
