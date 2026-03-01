// Copyright 2025 Quentin Cartier
#pragma once

#include <algorithm>

#include "udjourney/core/events/EventDispatcher.hpp"
#include "udjourney/core/events/HealthChangedEvent.hpp"
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
    explicit HealthComponent(
        int max_health, int initial_health = -1,
        udjourney::core::events::EventDispatcher* dispatcher = nullptr) :
        max_health_(max_health),
        current_health_(initial_health < 0 ? max_health : initial_health),
        dispatcher_(dispatcher) {
        current_health_ = std::clamp(current_health_, 0, max_health_);
        dispatch_health_changed_();
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
        const int old = current_health_;
        current_health_ = std::max(0, current_health_ - amount);
        if (current_health_ != old) {
            dispatch_health_changed_();
        }
    }

    /**
     * @brief Heal the actor
     * @param amount Healing amount (positive value)
     */
    void heal(int amount) {
        if (amount < 0) return;
        const int old = current_health_;
        current_health_ = std::min(max_health_, current_health_ + amount);
        if (current_health_ != old) {
            dispatch_health_changed_();
        }
    }

    /**
     * @brief Set health directly (clamped to valid range)
     */
    void set_health(int health) {
        const int old = current_health_;
        current_health_ = std::clamp(health, 0, max_health_);
        if (current_health_ != old) {
            dispatch_health_changed_();
        }
    }

    /**
     * @brief Set maximum health capacity
     */
    void set_max_health(int max_health) {
        const int old_max = max_health_;
        const int old_cur = current_health_;
        max_health_ = std::max(1, max_health);
        current_health_ = std::min(current_health_, max_health_);
        if (max_health_ != old_max || current_health_ != old_cur) {
            dispatch_health_changed_();
        }
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
    void dispatch_health_changed_() {
        if (!dispatcher_) return;
        udjourney::core::events::HealthChangedEvent ev(current_health_,
                                                       max_health_);
        dispatcher_->dispatch(ev);
    }

    IActor* owner_ = nullptr;
    int max_health_;
    int current_health_;
    udjourney::core::events::EventDispatcher* dispatcher_ = nullptr;
};
}  // namespace udjourney
