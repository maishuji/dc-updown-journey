// Copyright 2025 Quentin Cartier
#include "udjourney/Monster.hpp"

#include <cmath>
#include <iostream>

#include "udjourney/managers/TextureManager.hpp"
#include "udjourney/Player.hpp"
#include "udjourney/states/MonsterStates.hpp"

Monster::Monster(const IGame& game, Rectangle rect,
                 const std::string& sprite_sheet) :
    IActor(game), game_(game), rect_(rect) {
    // Load monster sprite sheet
    auto& texture_manager = TextureManager::get_instance();
    Texture2D sprite_texture = texture_manager.get_texture(sprite_sheet);

    // Initialize animations for different states
    // Using the new constructor with start_row and start_col

    // IDLE animation - row 0, columns 0-7
    anim_controller_.add_animation(0,
                                   "idle",
                                   SpriteAnim(sprite_texture,
                                              SPRITE_WIDTH,
                                              SPRITE_HEIGHT,
                                              FRAME_DURATION,
                                              FRAMES_PER_ANIMATION,
                                              true,
                                              0,
                                              0));

    // PATROL animation (same as idle for now) - row 0, columns 0-7
    anim_controller_.add_animation(1,
                                   "patrol",
                                   SpriteAnim(sprite_texture,
                                              SPRITE_WIDTH,
                                              SPRITE_HEIGHT,
                                              FRAME_DURATION / 2.0f,
                                              FRAMES_PER_ANIMATION,
                                              true,
                                              0,
                                              0));

    // CHASE animation (faster) - row 0, columns 0-7
    anim_controller_.add_animation(2,
                                   "chase",
                                   SpriteAnim(sprite_texture,
                                              SPRITE_WIDTH,
                                              SPRITE_HEIGHT,
                                              FRAME_DURATION / 4.0f,
                                              FRAMES_PER_ANIMATION,
                                              true,
                                              0,
                                              0));

    // ATTACK animation - row 1, columns 0-7 (if available)
    anim_controller_.add_animation(3,
                                   "attack",
                                   SpriteAnim(sprite_texture,
                                              SPRITE_WIDTH,
                                              SPRITE_HEIGHT,
                                              FRAME_DURATION / 2.0f,
                                              FRAMES_PER_ANIMATION,
                                              false,
                                              1,
                                              0));

    // HURT animation - row 2, columns 0-7 (if available)
    anim_controller_.add_animation(4,
                                   "hurt",
                                   SpriteAnim(sprite_texture,
                                              SPRITE_WIDTH,
                                              SPRITE_HEIGHT,
                                              FRAME_DURATION / 3.0f,
                                              4,
                                              false,
                                              2,
                                              0));

    // DEATH animation - row 3, columns 0-7 (if available)
    anim_controller_.add_animation(5,
                                   "death",
                                   SpriteAnim(sprite_texture,
                                              SPRITE_WIDTH,
                                              SPRITE_HEIGHT,
                                              FRAME_DURATION,
                                              FRAMES_PER_ANIMATION,
                                              false,
                                              3,
                                              0));

    // Set initial state
    anim_controller_.set_current_state(0);  // IDLE = 0

    // Register all monster states (using State pattern)
    states_["IDLE"] = std::make_unique<MonsterIdleState>();
    states_["PATROL"] = std::make_unique<MonsterPatrolState>();
    states_["CHASE"] = std::make_unique<MonsterChaseState>();
    states_["ATTACK"] = std::make_unique<MonsterAttackState>();
    states_["HURT"] = std::make_unique<MonsterHurtState>();
    states_["DEATH"] = std::make_unique<MonsterDeathState>();

    // Start in IDLE state
    current_state_ = states_["IDLE"].get();
    current_state_->enter(*this);
}

void Monster::draw() const {
    auto rect = rect_;
    const auto& game_rect = game_.get_rectangle();

    // Convert to screen coordinates
    rect.x -= game_rect.x;
    rect.y -= game_rect.y;

    // Draw current animation through the controller
    anim_controller_.draw(rect, !facing_right_);

    // Debug: Draw health bar
    if (health_ > 0.0f && health_ < max_health_) {
        Rectangle health_bar_bg = {rect.x, rect.y - 10, rect.width, 4};
        Rectangle health_bar = {
            rect.x, rect.y - 10, rect.width * (health_ / max_health_), 4};
        DrawRectangleRec(health_bar_bg, BLACK);
        DrawRectangleRec(health_bar, RED);
    }
}

void Monster::update(float delta) {
    // Update current state (handles AI logic via State pattern)
    if (current_state_) {
        current_state_->handleInput(*this);
        current_state_->update(*this, delta);
    }

    // Apply gravity
    apply_gravity(delta);

    // Update animations
    anim_controller_.update(delta);

    // Apply movement
    rect_.x += velocity_x_ * delta;
    rect_.y += velocity_y_;

    // Handle border collisions
    handle_border_collisions();

    // Update attack cooldown
    if (attack_cooldown_ > 0.0f) {
        attack_cooldown_ -= delta;
    }
}

void Monster::process_input() {
    // Monsters don't process player input
}

void Monster::change_state(const std::string& new_state) {
    auto it = states_.find(new_state);
    if (it != states_.end() && current_state_ != it->second.get()) {
        current_state_ = it->second.get();
        current_state_->enter(*this);

        // Map state names to animation indices
        if (new_state == "IDLE") {
            anim_controller_.set_current_state(0);
        } else if (new_state == "PATROL") {
            anim_controller_.set_current_state(1);
        } else if (new_state == "CHASE") {
            anim_controller_.set_current_state(2);
        } else if (new_state == "ATTACK") {
            anim_controller_.set_current_state(3);
        } else if (new_state == "HURT") {
            anim_controller_.set_current_state(4);
        } else if (new_state == "DEATH") {
            anim_controller_.set_current_state(5);
        }
    }
}

bool Monster::is_attacking() const {
    // Check if current animation is attack animation
    return anim_controller_.get_current_state_int() == 3;  // ATTACK = 3
}

void Monster::reverse_direction() {
    patrol_direction_right_ = !patrol_direction_right_;
    facing_right_ = patrol_direction_right_;
    velocity_x_ = -velocity_x_;
}

Player* Monster::find_player() const {
    // Use the IGame interface to access the player
    return game_.get_player();
}

void Monster::take_damage(float damage) {
    if (anim_controller_.get_current_state_int() == 5) {  // DEATH = 5
        return;
    }

    health_ -= damage;

    if (health_ <= 0.0f) {
        health_ = 0.0f;
        change_state("DEATH");
        velocity_x_ = 0.0f;
    } else {
        change_state("HURT");
        // Knockback
        velocity_x_ = facing_right_ ? -speed_ * 2.0f : speed_ * 2.0f;
    }
}

void Monster::set_patrol_range(float min_x, float max_x) {
    patrol_min_x_ = min_x;
    patrol_max_x_ = max_x;
}

void Monster::apply_gravity(float delta) {
    if (!grounded_) {
        velocity_y_ += GRAVITY;
        if (velocity_y_ > MAX_FALL_SPEED) {
            velocity_y_ = MAX_FALL_SPEED;
        }
    }
}

void Monster::handle_border_collisions() {
    const auto& game_rect = game_.get_rectangle();

    // Left border collision
    if (rect_.x < game_rect.x) {
        rect_.x = game_rect.x;
        velocity_x_ = 0.0f;

        // Reverse patrol direction if patrolling
        if (anim_controller_.get_current_state_int() == 1) {  // PATROL = 1
            patrol_direction_right_ = true;
        }
    }

    // Right border collision
    if (rect_.x + rect_.width > game_rect.x + game_rect.width) {
        rect_.x = game_rect.x + game_rect.width - rect_.width;
        velocity_x_ = 0.0f;

        // Reverse patrol direction if patrolling
        if (anim_controller_.get_current_state_int() == 1) {  // PATROL = 1
            patrol_direction_right_ = false;
        }
    }
}

void Monster::handle_collision(
    const std::vector<std::unique_ptr<IActor>>& actors) noexcept {
    const auto& game_rect = game_.get_rectangle();

    // Don't check collision if out of screen at top
    if (rect_.y < game_rect.y) {
        return;
    }

    const uint8_t PLATFORM_TYPE_ID = 1;

    grounded_ = false;

    for (const auto& actor : actors) {
        if (actor.get() == this) {
            continue;  // Skip self
        }

        if (check_collision(*actor)) {
            if (actor->get_group_id() == PLATFORM_TYPE_ID) {
                // Check if monster is above the platform (grounded)
                if (rect_.y < actor->get_rectangle().y) {
                    grounded_ = true;
                    velocity_y_ = 0.0f;
                }

                // Resolve collision
                Rectangle platform_rect = actor->get_rectangle();
                Rectangle intersect = GetCollisionRec(rect_, platform_rect);

                if (intersect.width < intersect.height) {
                    // Horizontal resolution
                    if (rect_.x < platform_rect.x) {
                        rect_.x -= intersect.width;  // Move monster left
                        velocity_x_ = 0.0f;

                        // Reverse patrol direction
                        if (anim_controller_.get_current_state_int() ==
                            1) {  // PATROL = 1
                            patrol_direction_right_ = false;
                        }
                    } else {
                        rect_.x += intersect.width;  // Move monster right
                        velocity_x_ = 0.0f;

                        // Reverse patrol direction
                        if (anim_controller_.get_current_state_int() ==
                            1) {  // PATROL = 1
                            patrol_direction_right_ = true;
                        }
                    }
                } else {
                    // Vertical resolution
                    if (rect_.y < platform_rect.y) {
                        rect_.y -= intersect.height;  // Move monster up
                        velocity_y_ = 0.0f;
                        grounded_ = true;
                    } else {
                        rect_.y += intersect.height;  // Move monster down
                        velocity_y_ = 0.0f;
                    }
                }
            }
        }
    }
}
