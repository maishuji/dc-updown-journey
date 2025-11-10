// Copyright 2025 Quentin Cartier
#pragma once
#include <raylib/raylib.h>

#include <vector>

enum class AnimationState { IDLE_WALK = 0, JUMP = 1, DASH = 2 };

class SpriteAnim {
 public:
    SpriteAnim() = default;
    SpriteAnim(Texture2D texture, int frame_width, int frame_height,
               float frame_time, int frames_per_row = 4, bool loop = true);
    
    // Constructor with starting sprite position
    SpriteAnim(Texture2D texture, int frame_width, int frame_height,
               float frame_time, int frames_per_row, bool loop,
               int start_row, int start_col);

    void update(float delta_time);
    void draw(Vector2 position, bool flip_horizontal = false) const;
    void draw_with_dest(Rectangle dest_rect,
                        bool flip_horizontal = false) const;
    void reset();
    void set_animation_state(AnimationState state);
    [[nodiscard]] Rectangle get_current_frame_rect() const;
    [[nodiscard]] bool is_finished() const noexcept { return finished_; }
    [[nodiscard]] AnimationState get_current_state() const noexcept {
        return current_state_;
    }

 private:
    Texture2D texture_ = {};
    int frame_width_ = 0;
    int frame_height_ = 0;
    float frame_time_ = 0.1f;
    int frames_per_row_ = 4;
    bool loop_ = true;

    int current_frame_ = 0;
    float elapsed_time_ = 0.0f;
    bool finished_ = false;
    AnimationState current_state_ = AnimationState::IDLE_WALK;

    int total_frames_per_row_ = 0;
    int start_row_ = 0;  // Starting row index in spritesheet
    int start_col_ = 0;  // Starting column index in spritesheet
};
