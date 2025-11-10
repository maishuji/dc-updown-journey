// Copyright 2025 Quentin Cartier
#include "udjourney/AnimSpriteController.hpp"

AnimSpriteController::AnimSpriteController() = default;

void AnimSpriteController::add_animation(PlayerState state,
                                         const std::string& name,
                                         SpriteAnim animation) {
    animations_[state] = std::move(animation);
}

void AnimSpriteController::set_current_state(PlayerState state) {
    if (current_state_ != state) {
        PlayerState old_state = current_state_;
        previous_state_ = current_state_;
        current_state_ = state;
        on_state_change(old_state, state);
    }
}

void AnimSpriteController::update(float delta_time) {
    auto it = animations_.find(current_state_);
    if (it != animations_.end()) {
        it->second.update(delta_time);
    }
}

void AnimSpriteController::draw(Rectangle dest_rect,
                                bool flip_horizontal) const {
    auto it = animations_.find(current_state_);
    if (it != animations_.end()) {
        // Use the controller's facing direction unless explicitly overridden
        bool should_flip = flip_horizontal || !facing_right_;
        it->second.draw_with_dest(dest_rect, should_flip);
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
    auto it = animations_.find(current_state_);
    if (it != animations_.end()) {
        return it->second.is_finished();
    }
    return false;
}
