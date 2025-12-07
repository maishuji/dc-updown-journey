// Copyright 2025 Quentin Cartier
#pragma once

#include <functional>
#include <memory>
#include <string>

#include "raylib/raylib.h"
#include "raylib/raymath.h"

#include "udjourney/interfaces/IActor.hpp"

namespace udjourney {

/**
 * @brief Trajectory type for projectiles
 */
enum class TrajectoryType {
    LINEAR,     // Straight line
    ARC,        // Parabolic arc (affected by gravity)
    SINE_WAVE,  // Sine wave pattern
    HOMING      // Follows target (future feature)
};

/**
 * @brief Configuration for a projectile type
 */
struct ProjectilePreset {
    std::string name;
    std::string texture_file;
    TrajectoryType trajectory = TrajectoryType::LINEAR;
    float speed = 200.0f;    // Pixels per second
    float gravity = 0.0f;    // For ARC trajectory
    float amplitude = 0.0f;  // For SINE_WAVE trajectory
    float frequency = 1.0f;  // For SINE_WAVE trajectory
    float lifetime = 5.0f;   // Seconds before auto-destruction
    Rectangle collision_bounds{0, 0, 8, 8};
    int damage = 1;
};

/**
 * @brief Projectile actor that can have different trajectories
 */
class Projectile : public IActor {
 public:
    Projectile(const IGame& game, const ProjectilePreset& preset,
               Vector2 start_pos, Vector2 direction);
    ~Projectile() override;

    void update(float delta) override;
    void draw() const override;
    void process_input() override {}  // Projectiles don't process input
    void set_rectangle(Rectangle rect) override { /* Not used */
    }
    [[nodiscard]] Rectangle get_rectangle() const override;
    [[nodiscard]] bool check_collision(const IActor& other) const override {
        return CheckCollisionRecs(get_rectangle(), other.get_rectangle());
    }
    [[nodiscard]] constexpr uint8_t get_group_id() const override {
        return 5;  // Projectile group
    }

    bool is_alive() const { return alive_; }
    void destroy() { alive_ = false; }
    int get_damage() const { return preset_.damage; }
    Rectangle get_bounding_box() const { return get_rectangle(); }

 private:
    ProjectilePreset preset_;
    Vector2 position_;
    Vector2 velocity_;
    Vector2 direction_;
    float elapsed_time_ = 0.0f;
    float distance_traveled_ = 0.0f;
    bool alive_ = true;
    Texture2D texture_{0};
    bool texture_loaded_ = false;
};

}  // namespace udjourney
