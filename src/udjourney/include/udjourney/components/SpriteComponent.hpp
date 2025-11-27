// Copyright 2025 Quentin Cartier
#pragma once

#include <raylib/raylib.h>
#include <string>
#include "udjourney/interfaces/IComponent.hpp"
#include "udjourney/interfaces/IActor.hpp"
#include "udjourney/AnimSpriteController.hpp"

/**
 * @brief Component that manages sprite rendering and animations
 *
 * Wraps AnimSpriteController for generic use across any actor type.
 */
class SpriteComponent : public IComponent {
 public:
    /**
     * @brief Construct sprite component
     * @param anim_controller Animation controller for this sprite
     */
    explicit SpriteComponent(AnimSpriteController anim_controller) :
        anim_controller_(anim_controller) {}

    ~SpriteComponent() override = default;

    void update(float delta) override { anim_controller_.update(delta); }

    void on_attach(IActor& actor) override { owner_ = &actor; }

    void on_detach(IActor& actor) override {
        (void)actor;
        owner_ = nullptr;
    }

    // ============= Rendering =============

    /**
     * @brief Draw the sprite at the actor's position
     */
    void draw() const {
        if (!owner_) return;

        Rectangle rect = owner_->get_rectangle();
        Vector2 position = {rect.x, rect.y};

        anim_controller_.draw(position, facing_right_);
    }

    /**
     * @brief Draw with specific flip state
     */
    void draw(Vector2 position, bool flip_horizontal) const {
        anim_controller_.draw(position, flip_horizontal);
    }

    // ============= Animation Control =============

    /**
     * @brief Change to a different animation
     * @param anim_id Animation ID from preset
     */
    void play_animation(int anim_id) {
        anim_controller_.set_animation(anim_id);
    }

    /**
     * @brief Check if current animation has finished playing
     */
    [[nodiscard]] bool is_animation_finished() const {
        return anim_controller_.is_animation_finished();
    }

    // ============= Direction =============

    void set_facing_right(bool right) { facing_right_ = right; }
    void flip_direction() { facing_right_ = !facing_right_; }

    [[nodiscard]] bool is_facing_right() const { return facing_right_; }

    // ============= Accessors =============

    AnimSpriteController& get_controller() { return anim_controller_; }
    [[nodiscard]] const AnimSpriteController& get_controller() const {
        return anim_controller_;
    }

 private:
    IActor* owner_ = nullptr;
    AnimSpriteController anim_controller_;
    bool facing_right_ = true;
};
