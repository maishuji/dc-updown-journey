// Copyright 2025 Quentin Cartier
#include "udjourney/SpriteAnim.hpp"

SpriteAnim::SpriteAnim(Texture2D texture, int frame_width, int frame_height,
                       float frame_time, int frames_per_row, bool loop) :
    texture_(texture),
    frame_width_(frame_width),
    frame_height_(frame_height),
    frame_time_(frame_time),
    frames_per_row_(frames_per_row),
    loop_(loop),
    start_row_(0),
    start_col_(0) {
    total_frames_per_row_ = frames_per_row;
}

SpriteAnim::SpriteAnim(Texture2D texture, int frame_width, int frame_height,
                       float frame_time, int frames_per_row, bool loop,
                       int start_row, int start_col) :
    texture_(texture),
    frame_width_(frame_width),
    frame_height_(frame_height),
    frame_time_(frame_time),
    frames_per_row_(frames_per_row),
    loop_(loop),
    start_row_(start_row),
    start_col_(start_col) {
    total_frames_per_row_ = frames_per_row;
}

void SpriteAnim::update(float delta_time) {
    if (finished_) return;

    elapsed_time_ += delta_time;
    if (elapsed_time_ >= frame_time_) {
        elapsed_time_ = 0.0f;
        current_frame_++;
        if (current_frame_ >= total_frames_per_row_) {
            if (loop_) {
                current_frame_ = 0;
            } else {
                current_frame_ = total_frames_per_row_ - 1;
                finished_ = true;
            }
        }
    }
}

void SpriteAnim::draw(Vector2 position, bool flip_horizontal) const {
    Rectangle dest_rect = {position.x,
                           position.y,
                           static_cast<float>(frame_width_),
                           static_cast<float>(frame_height_)};
    draw_with_dest(dest_rect, flip_horizontal);
}

void SpriteAnim::draw_with_dest(Rectangle dest_rect,
                                bool flip_horizontal) const {
    Rectangle src_rect = get_current_frame_rect();

    // Handle horizontal flipping
    if (flip_horizontal) {
        src_rect.width = -src_rect.width;
    }

    DrawTexturePro(
        texture_, src_rect, dest_rect, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
}

Rectangle SpriteAnim::get_current_frame_rect() const {
    // Calculate actual position in spritesheet
    int actual_col = start_col_ + current_frame_;
    int actual_row = start_row_ + static_cast<int>(current_state_);
    
    return Rectangle{static_cast<float>(actual_col * frame_width_),
                     static_cast<float>(actual_row * frame_height_),
                     static_cast<float>(frame_width_),
                     static_cast<float>(frame_height_)};
}

void SpriteAnim::set_animation_state(AnimationState state) {
    if (current_state_ != state) {
        current_state_ = state;
        current_frame_ = 0;  // Reset to first frame of new animation
        elapsed_time_ = 0.0f;
        finished_ = false;
    }
}

void SpriteAnim::reset() {
    current_frame_ = 0;
    elapsed_time_ = 0.0f;
    finished_ = false;
}
