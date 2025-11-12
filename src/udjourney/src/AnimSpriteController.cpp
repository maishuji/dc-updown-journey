// Copyright 2025 Quentin Cartier
#include "udjourney/AnimSpriteController.hpp"

#include <string>
#include <utility>

AnimSpriteController::AnimSpriteController() = default;

void AnimSpriteController::add_animation(PlayerState state,
                                         const std::string& name,
                                         SpriteAnim animation) {
    animations_[state] = std::move(animation);
    using_player_state_ = true;
}

void AnimSpriteController::add_animation(int state, const std::string& name,
                                         SpriteAnim animation) {
    animations_int_[state] = std::move(animation);
    using_player_state_ = false;
}

void AnimSpriteController::set_current_state(PlayerState state) {
    if (current_state_ != state) {
        PlayerState old_state = current_state_;
        previous_state_ = current_state_;
        current_state_ = state;
        using_player_state_ = true;
        on_state_change(old_state, state);
    }
}

void AnimSpriteController::set_current_state(int state) {
    if (current_state_int_ != state) {
        previous_state_int_ = current_state_int_;
        current_state_int_ = state;
        using_player_state_ = false;

        // Reset animation when state changes
        auto it = animations_int_.find(state);
        if (it != animations_int_.end()) {
            it->second.reset();
        }
    }
}

void AnimSpriteController::update(float delta_time) {
    if (using_player_state_) {
        auto it = animations_.find(current_state_);
        if (it != animations_.end()) {
            it->second.update(delta_time);
        }
    } else {
        auto it = animations_int_.find(current_state_int_);
        if (it != animations_int_.end()) {
            it->second.update(delta_time);
        }
    }
}

void AnimSpriteController::draw(Rectangle dest_rect,
                                bool flip_horizontal) const {
    if (using_player_state_) {
        auto it = animations_.find(current_state_);
        if (it != animations_.end()) {
            // Use the controller's facing direction unless explicitly
            // overridden
            bool should_flip = flip_horizontal || !facing_right_;
            it->second.draw_with_dest(dest_rect, should_flip);
        }
    } else {
        auto it = animations_int_.find(current_state_int_);
        if (it != animations_int_.end()) {
            bool should_flip = flip_horizontal || !facing_right_;
            it->second.draw_with_dest(dest_rect, should_flip);
        }
    }
}

void AnimSpriteController::on_state_change(PlayerState old_state,
                                           PlayerState new_state) {
    // Reset animation when state changes
    auto it = animations_.find(new_state);
    if (it != animations_.end()) {
        it->second.reset();  // Reset to frame 0
    }
}

bool AnimSpriteController::is_animation_finished() const {
    if (using_player_state_) {
        auto it = animations_.find(current_state_);
        if (it != animations_.end()) {
            return it->second.is_finished();
        }
    } else {
        auto it = animations_int_.find(current_state_int_);
        if (it != animations_int_.end()) {
            return it->second.is_finished();
        }
    }
    return false;
}
