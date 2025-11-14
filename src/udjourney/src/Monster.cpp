// Copyright 2025 Quentin Cartier
#include "udjourney/Monster.hpp"

#include <cmath>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <memory>

#include "udjourney/managers/TextureManager.hpp"
#include "udjourney/loaders/MonsterPresetLoader.hpp"
#include "udjourney/Player.hpp"
#include "udjourney/states/MonsterStates.hpp"
#include "udjourney/WorldBounds.hpp"

Monster::Monster(const IGame& game, Rectangle rect,
                 AnimSpriteController anim_controller) :
    IActor(game),
    game_(game),
    rect_(rect),
    anim_controller_(std::move(anim_controller)) {
    // Use default stats for now
    health_ = 100.0f;
    max_health_ = 100.0f;
    damage_ = 10.0f;
    speed_ = 50.0f;
    chase_range_ = 200.0f;
    attack_range_ = 50.0f;

    // Set initial state
    anim_controller_.set_current_state(ANIM_IDLE);

    // Register all monster states (using State pattern)
    states_["idle"] = std::make_unique<MonsterIdleState>();
    states_["patrol"] = std::make_unique<MonsterPatrolState>();
    states_["chase"] = std::make_unique<MonsterChaseState>();
    states_["attack"] = std::make_unique<MonsterAttackState>();
    states_["hurt"] = std::make_unique<MonsterHurtState>();
    states_["death"] = std::make_unique<MonsterDeathState>();

    // Initialize preset to nullptr for now (will be set later if needed)
    preset_ = nullptr;
    preset_name_ = "";

    // Start in default idle state (preset can override this later)
    std::string initial_state = "idle";
    if (states_.find(initial_state) != states_.end()) {
        current_state_ = states_[initial_state].get();
        current_state_->enter(*this);
        std::cout << "Monster initialized with default state: " << initial_state
                  << std::endl;
    } else {
        std::cerr << "CRITICAL ERROR: Default 'idle' state not found! "
                     "Available states: ";
        for (const auto& state_pair : states_) {
            std::cerr << state_pair.first << " ";
        }
        std::cerr << std::endl;
        throw std::runtime_error(
            "Monster initialization failed: No valid default state found");
    }
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
        std::string old_state_name = "unknown";
        // Find current state name for logging
        for (const auto& state_pair : states_) {
            if (state_pair.second.get() == current_state_) {
                old_state_name = state_pair.first;
                break;
            }
        }

        std::string monster_name = preset_ ? preset_->name : "default_monster";
        std::cout << "Monster '" << monster_name
                  << "' changing state: " << old_state_name << " -> "
                  << new_state << std::endl;

        current_state_ = it->second.get();
        current_state_->enter(*this);

        // Map state names to animation indices (using lowercase)
        if (new_state == "idle") {
            anim_controller_.set_current_state(ANIM_IDLE);
        } else if (new_state == "patrol") {
            anim_controller_.set_current_state(ANIM_PATROL);
        } else if (new_state == "chase") {
            anim_controller_.set_current_state(ANIM_CHASE);
        } else if (new_state == "attack") {
            anim_controller_.set_current_state(ANIM_ATTACK);
        } else if (new_state == "hurt") {
            anim_controller_.set_current_state(ANIM_HURT);
        } else if (new_state == "death") {
            anim_controller_.set_current_state(ANIM_DEATH);
        }
    } else if (it == states_.end()) {
        std::string monster_name = preset_ ? preset_->name : "default_monster";
        std::cerr << "ERROR: Attempted to change to invalid state '"
                  << new_state << "' for monster '" << monster_name
                  << "'. Available states: ";
        for (const auto& state_pair : states_) {
            std::cerr << state_pair.first << " ";
        }
        std::cerr << std::endl;
    }
}

bool Monster::is_attacking() const {
    // Check if current animation is attack animation
    return anim_controller_.get_current_state_int() == ANIM_ATTACK;
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
    if (anim_controller_.get_current_state_int() == ANIM_DEATH) {
        return;
    }

    health_ -= damage;

    if (health_ <= 0.0f) {
        health_ = 0.0f;
        change_state("death");
        velocity_x_ = 0.0f;
    } else {
        change_state("hurt");
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
    // Use WorldBounds for more accurate boundary collision
    const auto& world_bounds = game_.get_world_bounds();
    auto collision = world_bounds.check_border_collision(rect_);

    if (collision.hit_left) {
        rect_ = collision.corrected_rect;
        velocity_x_ = 0.0f;

        // Reverse patrol direction if patrolling
        if (anim_controller_.get_current_state_int() == 1) {  // PATROL = 1
            patrol_direction_right_ = true;
        }
    }

    if (collision.hit_right) {
        rect_ = collision.corrected_rect;
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

bool Monster::is_wall_ahead() const {
    // Simple wall detection - check if we're at patrol boundaries
    if (facing_right_ && rect_.x >= patrol_max_x_) {
        return true;
    } else if (!facing_right_ && rect_.x <= patrol_min_x_) {
        return true;
    }
    return false;
}

bool Monster::is_animation_finished() const {
    // For now, we'll use a simple time-based approach
    // This could be enhanced to check actual animation frame completion
    return anim_controller_.is_animation_finished();
}

void Monster::load_preset(const std::string& preset_name) {
    try {
        std::cout << "Loading monster preset: " << preset_name << std::endl;

        // Load preset from JSON file (MonsterPresetLoader handles the path
        // construction)
        std::string preset_filename = preset_name + ".json";
        preset_ = udjourney::MonsterPresetLoader::load_preset(preset_filename);
        preset_name_ = preset_name;

        std::cout << "Monster preset loaded successfully: " << preset_name
                  << std::endl;

        // Apply preset stats
        max_health_ = preset_->stats.max_health;
        health_ = max_health_;  // Reset to full health
        speed_ = preset_->stats.movement_speed;
        damage_ = preset_->stats.damage;

        // Apply behavior settings
        chase_range_ = preset_->behavior.chase_range;
        attack_range_ = preset_->behavior.attack_range;

        // Change to preset's initial state if different from current
        std::string target_state = preset_->state_config.initial_state;
        if (target_state != "idle") {  // Only change if not already idle
            change_state(target_state);
        }

        std::cout << "Monster configured with preset '" << preset_->name
                  << "' (HP: " << max_health_ << ", Speed: " << speed_
                  << ", State: " << target_state << ")" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Failed to load monster preset '" << preset_name
                  << "': " << e.what() << std::endl;
        std::cerr << "Monster will continue with default settings."
                  << std::endl;
    }
}
