// Copyright 2025 Quentin Cartier
#include "udjourney/Projectile.hpp"

#include <cmath>
#include <iostream>
#include <string>

#include <udj-core/CoreUtils.hpp>

#include "udjourney/interfaces/IGame.hpp"

namespace udjourney {

Projectile::Projectile(const IGame& game, const ProjectilePreset& preset,
                       Vector2 start_pos, Vector2 direction) :
    IActor(game), preset_(preset), position_(start_pos), direction_(direction) {
    // Normalize direction
    float length =
        std::sqrt(direction_.x * direction_.x + direction_.y * direction_.y);
    if (length > 0.0f) {
        direction_.x /= length;
        direction_.y /= length;
    }

    // Initialize velocity based on trajectory type
    velocity_ = {direction_.x * preset_.speed, direction_.y * preset_.speed};

    // Load texture
    std::string texture_path =
        udj::core::filesystem::get_assets_path(preset_.texture_file);
    if (udj::core::filesystem::file_exists(texture_path)) {
        texture_ = LoadTexture(texture_path.c_str());
        texture_loaded_ = true;
    } else {
        std::cerr << "Warning: Projectile texture not found: " << texture_path
                  << std::endl;
    }
}

Projectile::~Projectile() {
    if (texture_loaded_) {
        UnloadTexture(texture_);
    }
}

void Projectile::update(float delta) {
    if (!alive_) return;

    float dt = delta > 0.0f ? delta : GetFrameTime();
    elapsed_time_ += dt;

    // Check lifetime
    if (elapsed_time_ >= preset_.lifetime) {
        alive_ = false;
        return;
    }

    // Update position based on trajectory type
    switch (preset_.trajectory) {
        case TrajectoryType::LINEAR:
            position_.x += velocity_.x * dt;
            position_.y += velocity_.y * dt;
            break;

        case TrajectoryType::ARC:
            // Apply gravity
            velocity_.y += preset_.gravity * dt;
            position_.x += velocity_.x * dt;
            position_.y += velocity_.y * dt;
            break;

        case TrajectoryType::SINE_WAVE: {
            // Move forward and oscillate perpendicular to direction
            distance_traveled_ += preset_.speed * dt;
            float wave_offset =
                preset_.amplitude *
                std::sin(elapsed_time_ * preset_.frequency * 2.0f * PI);

            // Perpendicular direction
            Vector2 perp = {-direction_.y, direction_.x};

            position_.x = position_.x + direction_.x * preset_.speed * dt +
                          perp.x * wave_offset * dt;
            position_.y = position_.y + direction_.y * preset_.speed * dt +
                          perp.y * wave_offset * dt;
            break;
        }

        case TrajectoryType::HOMING:
            // TODO(user): Implement homing logic (requires target)
            position_.x += velocity_.x * dt;
            position_.y += velocity_.y * dt;
            break;
    }
}

void Projectile::draw() const {
    if (!alive_) return;

    // Convert to screen coordinates
    const auto& game = get_game();
    Vector2 screen_pos = position_;
    screen_pos.x -= game.get_rectangle().x;
    screen_pos.y -= game.get_rectangle().y;

    if (texture_loaded_) {
        if (preset_.use_atlas && preset_.source_rect.width > 0.0f &&
            preset_.source_rect.height > 0.0f) {
            DrawTextureRec(texture_, preset_.source_rect, screen_pos, WHITE);
        } else {
            DrawTexture(texture_,
                        static_cast<int>(screen_pos.x),
                        static_cast<int>(screen_pos.y),
                        WHITE);
        }
    } else {
        // Draw a red circle if no texture
        DrawCircle(static_cast<int>(screen_pos.x),
                   static_cast<int>(screen_pos.y),
                   6.0f,
                   RED);
    }
}

Rectangle Projectile::get_rectangle() const {
    return Rectangle{position_.x + preset_.collision_bounds.x,
                     position_.y + preset_.collision_bounds.y,
                     preset_.collision_bounds.width,
                     preset_.collision_bounds.height};
}

}  // namespace udjourney
