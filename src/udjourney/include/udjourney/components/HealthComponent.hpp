// Copyright 2025 Quentin Cartier
#pragma once

#include <algorithm>
#include "udjourney/interfaces/IComponent.hpp"
#include "udjourney/interfaces/IActor.hpp"
namespace udjourney {

/**
 * @brief Component that manages actor health, damage, and healing
 *
 * Provides generic health management that can be used by any actor type.
 * Automatically clamps health between 0 and max_health.
 */
class HealthComponent : public IComponent {
 public:
    /**
     * @brief Construct health component with initial values
     * @param max_health Maximum health capacity
     * @param initial_health Starting health (defaults to max if not specified)
     */
    explicit HealthComponent(int max_health, int initial_health = -1) :
        max_health_(max_health),
        current_health_(initial_health < 0 ? max_health : initial_health) {
        current_health_ = std::clamp(current_health_, 0, max_health_);
    }

    ~HealthComponent() override = default;

    void update(float delta) override {
        // Health component is passive, no update needed
        (void)delta;
    }

    void on_attach(IActor& actor) override { owner_ = &actor; }

    void on_detach(IActor& actor) override {
        (void)actor;
        owner_ = nullptr;
    }

    // ============= Health Management =============

    /**
     * @brief Apply damage to the actor
     * @param amount Damage amount (positive value)
     */
    void take_damage(int amount) {
        if (amount < 0) return;
        current_health_ = std::max(0, current_health_ - amount);
    }

    /**
     * @brief Heal the actor
     * @param amount Healing amount (positive value)
     */
    void heal(int amount) {
        if (amount < 0) return;
        current_health_ = std::min(max_health_, current_health_ + amount);
    }

    /**
     * @brief Set health directly (clamped to valid range)
     */
    void set_health(int health) {
        current_health_ = std::clamp(health, 0, max_health_);
    }

    /**
     * @brief Set maximum health capacity
     */
    void set_max_health(int max_health) {
        max_health_ = std::max(1, max_health);
        current_health_ = std::min(current_health_, max_health_);
    }

    // ============= Accessors =============

    [[nodiscard]] int get_health() const { return current_health_; }
    [[nodiscard]] int get_max_health() const { return max_health_; }
    [[nodiscard]] bool is_alive() const { return current_health_ > 0; }
    [[nodiscard]] float get_health_percentage() const {
        return static_cast<float>(current_health_) /
               static_cast<float>(max_health_);
    }

 private:
    IActor* owner_ = nullptr;
    int max_health_;
    int current_health_;
};
}  // namespace udjourney
