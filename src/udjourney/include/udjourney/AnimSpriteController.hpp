// Copyright 2025 Quentin Cartier
#pragma once
#include <raylib/raylib.h>
#include <unordered_map>
#include <string>
#include "udjourney/SpriteAnim.hpp"

enum class PlayerState { IDLE, RUNNING, JUMPING, DASHING, FALLING };

class AnimSpriteController {
 public:
    AnimSpriteController();

    // Animation management
    void add_animation(PlayerState state, const std::string& name,
                       SpriteAnim animation);
    void set_current_state(PlayerState state);
    void update(float delta_time);
    void draw(Rectangle dest_rect, bool flip_horizontal = false) const;

    // State queries
    PlayerState get_current_state() const { return current_state_; }
    bool is_animation_finished() const;

    // Configuration
    void set_facing_right(bool facing_right) { facing_right_ = facing_right; }
    bool is_facing_right() const { return facing_right_; }

 private:
    PlayerState current_state_ = PlayerState::IDLE;
    PlayerState previous_state_ = PlayerState::IDLE;
    std::unordered_map<PlayerState, SpriteAnim> animations_;
    bool facing_right_ = true;

    void on_state_change(PlayerState old_state, PlayerState new_state);
};
