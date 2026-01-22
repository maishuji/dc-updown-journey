// Copyright 2025 Quentin Cartier
#include "udjourney/Monster.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <memory>

#include "udj-core/Logger.hpp"
#include "udjourney/managers/TextureManager.hpp"
#include "udjourney/core/events/ScoreEvent.hpp"
#include "udjourney/core/events/EventDispatcher.hpp"
#include "udjourney/loaders/MonsterPresetLoader.hpp"
#include "udjourney/Player.hpp"
#include "udjourney/states/MonsterStates.hpp"
#include "udjourney/WorldBounds.hpp"

using udj::core::Logger;

namespace udjourney {
Monster::Monster(const IGame& game, Rectangle rect,
                 AnimSpriteController anim_controller,
                 udjourney::core::events::EventDispatcher& dispatcher,
                 const scene::LevelPhysicsConfig& physics_config) :
    IActor(game),
    game_(game),
    rect_(rect),
    anim_controller_(std::move(anim_controller)),
    dispatcher_(dispatcher),
    physics_config_(physics_config) {
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
        Logger::info("Monster initialized with default state: " +
                     initial_state);
    } else {
        Logger::error(
            "CRITICAL ERROR: Default 'idle' state not found! "
            "Available states: ");
        for (const auto& state_pair : states_) {
            Logger::error(state_pair.first);
        }
        throw std::runtime_error(
            "Monster initialization failed: No valid default state found");
    }
}

Rectangle Monster::get_rectangle() const {
    // If animation has collision bounds, use those
    if (anim_controller_.has_collision_bounds()) {
        auto bounds = anim_controller_.get_collision_bounds();
        return Rectangle{rect_.x + bounds.offset_x,
                         rect_.y + bounds.offset_y,
                         bounds.width,
                         bounds.height};
    }
    // Otherwise, use the default rectangle
    return rect_;
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
        Logger::info("Monster '" + monster_name + "' changing state: " +
                     old_state_name + " -> " + new_state);

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
        Logger::error("ERROR: Attempted to change to invalid state '" +
                      new_state + "' for monster '" + monster_name +
                      "'. Available states: ");
        for (const auto& state_pair : states_) {
            Logger::error(state_pair.first + " ");
        }
    }
}

bool Monster::is_attacking() const {
    // Check if current animation is attack animation
    return anim_controller_.get_current_state_int() == ANIM_ATTACK;
}

bool Monster::is_dying() const {
    // Check if current animation is death animation
    return anim_controller_.get_current_state_int() == ANIM_DEATH;
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
    udj::core::Logger::info(
        "Monster::take_damage called! Damage: % Current health: %",
        damage,
        health_);

    if (anim_controller_.get_current_state_int() == ANIM_DEATH) {
        udj::core::Logger::info(
            "Monster already in death animation, ignoring damage");
        return;
    }

    health_ -= damage;
    udj::core::Logger::info("After damage, health: %", health_);
    if (health_ <= 0.0f) {
        health_ = 0.0f;
        udj::core::Logger::info("Monster health <= 0, entering death state");

        // Check if death state exists before trying to use it
        if (states_.find("death") != states_.end()) {
            udj::core::Logger::info("Death state found, changing to death");
            change_state("death");
        } else {
            // No death state defined, just mark as consumed
            udj::core::Logger::info(
                "Monster has no death state defined, marking as consumed");
            set_state(ActorState::CONSUMED);
        }
        velocity_x_ = 0.0f;
    } else {
        udj::core::Logger::info("Monster still alive, health: %", health_);
        // Check if hurt state exists
        if (states_.find("hurt") != states_.end()) {
            change_state("hurt");
        }
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
        velocity_y_ += physics_config_.gravity;
        if (velocity_y_ > physics_config_.terminal_velocity) {
            velocity_y_ = physics_config_.terminal_velocity;
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
    const uint8_t MONSTER_TYPE_ID = 3;

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
            } else if (actor->get_group_id() == MONSTER_TYPE_ID) {
                // Enemy-enemy collision: prevent overlapping
                Rectangle other_rect = actor->get_rectangle();
                Rectangle intersect = GetCollisionRec(rect_, other_rect);

                // Only resolve horizontally to avoid interfering with
                // gravity
                if (intersect.width > 0 && intersect.height > 0) {
                    // Push monsters apart horizontally
                    if (rect_.x < other_rect.x) {
                        // This monster is on the left, push it left
                        rect_.x -= intersect.width / 2.0f;
                    } else {
                        // This monster is on the right, push it right
                        rect_.x += intersect.width / 2.0f;
                    }

                    // Optionally reverse direction when colliding with
                    // another enemy (prevents them from constantly pushing
                    // into each other)
                    if (anim_controller_.get_current_state_int() ==
                        1) {  // PATROL
                        patrol_direction_right_ = !patrol_direction_right_;
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
        Logger::info("Loading monster preset: " + preset_name);

        // Load preset from JSON file (MonsterPresetLoader handles the path
        // construction)
        std::string preset_filename = preset_name + ".json";
        preset_ = udjourney::MonsterPresetLoader::load_preset(preset_filename);
        preset_name_ = preset_name;

        Logger::info("Monster preset loaded successfully: " + preset_name);

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

        Logger::info("Monster configured with preset '" + preset_->name +
                         "' (HP: %, Speed: %, State: %",
                     max_health_,
                     speed_,
                     target_state);
    } catch (const std::exception& e) {
        Logger::error("Failed to load monster preset '" + preset_name +
                      "': " + e.what());
        Logger::error("Monster will continue with default settings.");
    }
}

void Monster::award_kill_points() const {
    // Award points based on monster type or default
    int points = 100;  // Default points for killing a monster

    // Could vary points based on monster difficulty in the future
    if (preset_ != nullptr) {
        // For now, use a simple calculation based on max health
        points =
            static_cast<int>(max_health_ / 10.0f) * 10;  // 10 points per 10 HP
        if (points < 50) points = 50;                    // Minimum points
        if (points > 500) points = 500;                  // Maximum points
    }

    // Use the EventDispatcher pattern to notify about scoring (like Player
    // does)
    udjourney::core::events::ScoreEvent score_event{points};
    dispatcher_.dispatch(score_event);

    udj::core::Logger::info("Monster awarded % points for kill", points);
}

// Observable methods implementation (same as Player)
void Monster::add_observer(IObserver* observer) {
    observers.push_back(observer);
}

void Monster::remove_observer(IObserver* observer) {
    observers.erase(std::remove(observers.begin(), observers.end(), observer),
                    observers.end());
}

void Monster::notify(const std::string& event) {
    for (auto* observer : observers) {
        observer->on_notify(event);
    }
}

}  // namespace udjourney
