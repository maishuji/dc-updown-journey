// Copyright 2025 Quentin Cartier
#pragma once

#include <raylib/raylib.h>
#include "udjourney/interfaces/IComponent.hpp"
#include "udjourney/interfaces/IActor.hpp"

/**
 * @brief Component that manages actor movement and physics
 *
 * Handles velocity, gravity, and position updates.
 * Can be used for both player-controlled and AI-controlled movement.
 */
class MovementComponent : public IComponent {
 public:
    /**
     * @brief Construct movement component
     * @param speed Base movement speed in pixels per second
     * @param apply_gravity Whether gravity should affect this actor
     */
    explicit MovementComponent(float speed = 100.0f,
                               bool apply_gravity = true) :
        speed_(speed), apply_gravity_(apply_gravity) {}

    ~MovementComponent() override = default;

    void update(float delta) override {
        if (!owner_) return;

        // Apply gravity if enabled
        if (apply_gravity_ && !grounded_) {
            velocity_.y +=
                GRAVITY * delta * 60.0f;  // Scale by 60 for frame-independence
            if (velocity_.y > MAX_FALL_SPEED) {
                velocity_.y = MAX_FALL_SPEED;
            }
        }

        // Update position based on velocity
        Rectangle rect = owner_->get_rectangle();
        rect.x += velocity_.x * delta;
        rect.y += velocity_.y * delta;
        owner_->set_rectangle(rect);
    }

    void on_attach(IActor& actor) override { owner_ = &actor; }

    void on_detach(IActor& actor) override {
        (void)actor;
        owner_ = nullptr;
    }

    // ============= Movement Control =============

    /**
     * @brief Set horizontal movement direction
     * @param direction -1 for left, 0 for stop, 1 for right
     */
    void move_horizontal(float direction) { velocity_.x = direction * speed_; }

    /**
     * @brief Apply a jump impulse (if grounded)
     * @param jump_force Upward force to apply
     */
    void jump(float jump_force = 10.0f) {
        if (grounded_) {
            velocity_.y = -jump_force;
            grounded_ = false;
        }
    }

    /**
     * @brief Stop all movement
     */
    void stop() { velocity_ = {0, 0}; }

    /**
     * @brief Stop horizontal movement only
     */
    void stop_horizontal() { velocity_.x = 0; }

    // ============= Physics =============

    void set_grounded(bool grounded) {
        grounded_ = grounded;
        if (grounded) {
            velocity_.y = 0;
        }
    }

    void set_velocity(Vector2 vel) { velocity_ = vel; }
    void set_velocity_x(float vx) { velocity_.x = vx; }
    void set_velocity_y(float vy) { velocity_.y = vy; }

    // ============= Configuration =============

    void set_speed(float speed) { speed_ = speed; }
    void set_gravity_enabled(bool enabled) { apply_gravity_ = enabled; }

    // ============= Accessors =============

    [[nodiscard]] Vector2 get_velocity() const { return velocity_; }
    [[nodiscard]] float get_speed() const { return speed_; }
    [[nodiscard]] bool is_grounded() const { return grounded_; }
    [[nodiscard]] bool is_moving() const {
        return velocity_.x != 0 || velocity_.y != 0;
    }

 private:
    IActor* owner_ = nullptr;
    Vector2 velocity_ = {0, 0};
    float speed_;
    bool apply_gravity_;
    bool grounded_ = false;

    // Physics constants
    static constexpr float GRAVITY = 1.0f;
    static constexpr float MAX_FALL_SPEED = 10.0f;
};
