// Copyright 2025 Quentin Cartier
#include "udjourney/Player.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <memory>
#include <utility>
#include <vector>

#include "raylib/raymath.h"
#include "raylib/rlgl.h"

#include <udj-core/CoreUtils.hpp>
#include <udj-core/Logger.hpp>

#include "udjourney/Monster.hpp"
#include "udjourney/Projectile.hpp"
#include "udjourney/ProjectilePresetLoader.hpp"
#include "udjourney/WorldBounds.hpp"
#include "udjourney/components/HealthComponent.hpp"
#include "udjourney/core/events/ScoreEvent.hpp"
#include "udjourney/managers/TextureManager.hpp"
#include "udjourney/platform/Platform.hpp"

#include "udjourney/core/events/WeaponSelectedEvent.hpp"
namespace udjourney {
Player::~Player() = default;

namespace {

const float kDashTimerDefault = 0.2F;
const float kDashCooldownDefault = 1.0F;
const float kMoveSpeedYDefault = 5.0F;
const float kMoveSpeedXDefault = 3.0F;
const float kDashSpeed = 15.0F;
const float kJumpExhaustion = 0.1F;
const float kJumpStrength = -8.0F;  // Initial jump velocity (negative = up)

struct InputMapping {
    std::function<bool()> left_pressed;
    std::function<bool()> right_pressed;
    std::function<bool()> up_pressed;
    std::function<bool()> down_pressed;
    std::function<bool()> jump_pressed;
    std::function<bool()> dash_pressed;
    std::function<bool()> shoot_pressed;

    InputMapping() {
#ifdef PLATFORM_DREAMCAST
        left_pressed = []() {
            return IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
        };
        right_pressed = []() {
            return IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
        };
        up_pressed = []() { return IsKeyPressed(KEY_UP); };
        down_pressed = []() {
            return IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
        };
        jump_pressed = []() {
            return IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
        };
        dash_pressed = []() {
            return IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
        };
        shoot_pressed = []() { return IsMouseButtonDown(MOUSE_LEFT_BUTTON); };
#else
        left_pressed = []() { return IsKeyDown(KEY_A); };
        right_pressed = []() { return IsKeyDown(KEY_D); };
        up_pressed = []() { return IsKeyDown(KEY_W); };
        down_pressed = []() { return IsKeyDown(KEY_S); };
        jump_pressed = []() { return IsKeyDown(KEY_SPACE); };
        dash_pressed = []() { return IsKeyDown(KEY_LEFT_SHIFT); };
        shoot_pressed = []() {
            return IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
        };

#endif
    }
} input_mapping;
}  // namespace

struct Player::PImpl {
    bool grounded = false;
    bool colliding = false;
    bool jumping = false;
    bool dashing = false;
    bool dashable = true;
    float velocity_y = 0.0F;  // Vertical velocity
    float dash_timer = 0.0F;
    float dash_cooldown = 0.0F;
    Platform *grounded_src = nullptr;
    int max_jumps = 2;  // Allow double jump
    int current_jumps = 0;  // Track how many jumps have been used
};

Player::Player(const IGame &iGame, Rectangle iRect,
               udjourney::core::events::EventDispatcher &ioDispatcher,
               AnimSpriteController anim_controller,
               const scene::LevelPhysicsConfig &physics_config) :
    IActor(iGame),
    r(iRect),
    m_pimpl(std::make_unique<Player::PImpl>()),
    anim_controller_(std::move(anim_controller)),
    m_dispatcher(ioDispatcher),
    m_physics_config(physics_config) {
    if (m_texture.id == 0) {
        auto &texture_manager = TextureManager::get_instance();
        m_texture = texture_manager.get_texture("placeholder.png");
    }

    // Add health component - 3 hearts = 6 half-hearts
    add_component(std::make_unique<HealthComponent>(6, 6, &m_dispatcher));
}

void Player::draw() const {
    auto rect = r;
    const auto &game = get_game();
    // Convert to screen coordinates
    rect.x -= game.get_rectangle().x;
    rect.y -= game.get_rectangle().y;

    // Draw current animation through the controller
    anim_controller_.draw(rect, !m_facing_right);

    if (is_invincible()) {
        // Draw a yellow border around the player when invincible
        DrawRectangleLinesEx(rect, 3.0F, YELLOW);
    }
}

void Player::update(float iDelta) {
    if (!m_pimpl) {
        udj::core::Logger::error("Player::update - m_pimpl is null!");
        return;
    }

    // Update all components
    update_components(iDelta);

    // Check if player died - stop all further updates
    if (auto *health = get_component<HealthComponent>()) {
        if (!health->is_alive()) {
            udj::core::Logger::debug("Player is dead - stopping update");
            // Don't notify here - let it happen naturally in collision or
            // elsewhere Just stop updating
            return;
        }
    }

    if (m_invincibility_timer > 0.0F) {
        m_invincibility_timer -= iDelta;
    } else {
        m_invincibility_timer = 0.0F;
    }

    // Update shoot cooldown
    if (shoot_cooldown_ > 0.0f) {
        shoot_cooldown_ -= iDelta;
    }

    // Update animation state and controller
    update_animation_state();
    anim_controller_.update(iDelta);

    // Apply gravity
    m_pimpl->velocity_y += m_physics_config.gravity;
    // Clamp to terminal velocity
    if (m_pimpl->velocity_y > m_physics_config.terminal_velocity) {
        m_pimpl->velocity_y = m_physics_config.terminal_velocity;
    }

    // Apply vertical velocity to position
    r.y += m_pimpl->velocity_y;

    // Follow platform movement when grounded
    if (m_pimpl->grounded && m_pimpl->grounded_src != nullptr) {
        r.x += m_pimpl->grounded_src->get_dx();
    }

    // Dash timers
    if (m_pimpl->dashing) {
        m_pimpl->dash_timer -= iDelta;
        if (m_pimpl->dash_timer <= 0.0F) {
            m_pimpl->dashing = false;
        }
    }
    if (!m_pimpl->dashing && !m_pimpl->dashable) {
        if (m_pimpl->dash_cooldown > 0.0F) {
            m_pimpl->dash_cooldown -= iDelta;
        } else {
            m_pimpl->dashable = true;
            notify("4;1");
        }
    }

    const auto &gameRect = get_game().get_rectangle();

    // Handle world bounds collision
    const auto &world_bounds = get_game().get_world_bounds();
    auto collision = world_bounds.check_border_collision(r);

    if (collision.hit_left || collision.hit_right) {
        // Apply corrected position for horizontal boundaries
        r = collision.corrected_rect;
    }

    // Gameover it the player is out of the screen at the bottom
    if (r.y > gameRect.y + gameRect.height) {
        r.y = gameRect.y + r.height;
        notify("12");  // Game over
    }
}

void Player::process_input() {
    if (!m_pimpl) {
        udj::core::Logger::error("Player::process_input - m_pimpl is null!");
        return;
    }

    if (input_mapping.left_pressed()) {
        r.x -= m_pimpl->dashing ? kDashSpeed : kMoveSpeedXDefault;
        m_facing_right = false;  // Update facing direction
        m_facing_right = false;  // Update facing direction
    }
    if (input_mapping.right_pressed()) {
        r.x += m_pimpl->dashing ? kDashSpeed : kMoveSpeedXDefault;
        m_facing_right = true;  // Update facing direction
        m_facing_right = true;  // Update facing direction
    }

    // A to Jump (with double jump support)
    if (input_mapping.jump_pressed()) {
        // Allow jump if: grounded OR still have jumps remaining
        if (!m_pimpl->jumping && m_pimpl->current_jumps < m_pimpl->max_jumps) {
            m_pimpl->jumping = true;
            m_pimpl->velocity_y = kJumpStrength;  // Apply initial jump velocity
            m_pimpl->grounded = false;            // Player is now airborne
            m_pimpl->current_jumps++;             // Increment jump counter
        }
    } else {
        m_pimpl->jumping = false;
    }

    // DPAD down
    if (input_mapping.down_pressed()) {
        r.y += kMoveSpeedYDefault;
    }

    // Dash input
    if ((input_mapping.dash_pressed()) && m_pimpl->dash_cooldown <= 0.0F) {
        m_pimpl->dashing = true;
        m_pimpl->dash_timer = kDashTimerDefault;  // Dash lasts 0.2 seconds
        m_pimpl->dash_cooldown =
            kDashCooldownDefault;  // Then 1 second cooldown
        m_pimpl->dashable =
            false;  // Player can't dash again until cooldown is over
        notify("4;0");
    }

    // Test attack input (X key) - damage nearby monsters
    if (IsKeyPressed(KEY_X)) {
        attack_nearby_monsters();
    }
}

void Player::resolve_collision(const IActor &iActor) noexcept {
    auto platformRect = iActor.get_rectangle();
    Rectangle intersect = GetCollisionRec(r, platformRect);

    if (intersect.width < intersect.height) {
        // Horizontal resolution
        if (r.x < platformRect.x) {
            r.x -= intersect.width;  // Move player to the left
        } else {
            r.x += intersect.width;  // Move player to the right
        }
    } else {
        // Vertical resolution
        if (r.y < platformRect.y) {
            r.y -= intersect.height;  // Move player up
        } else {
            r.y += intersect.height;  // Move player down
        }
    }
}

/**
 * @brief Hangle collision with other actors
 *
 * This function is called from the main game loop to handle collision
 *
 * @param platforms A vector of unique pointers to IActor objects
 * representing the platforms
 * @return void
 * @throws none
 */
void Player::handle_collision(
    const std::vector<std::unique_ptr<IActor>> &platforms) noexcept {
    udj::core::Logger::debug("Player::handle_collision called");

    // Defensive check: ensure m_pimpl is valid
    if (!m_pimpl) {
        udj::core::Logger::error(
            "Player::handle_collision - m_pimpl is null! Player object may "
            "have been moved or corrupted.");
        return;
    }

    const auto &gameRect = get_game().get_rectangle();

    // Dont check collision if the player is out of the screen at the top
    if (this->get_rectangle().y < gameRect.y) {
        return;
    }

    const uint8_t PLATFORM_TYPE_ID = 1;
    const uint8_t BONUS_TYPE_ID = 2;
    const uint8_t MONSTER_TYPE_ID = 3;

    bool tmp_colliding = false;
    bool tmp_grounded = false;
    Platform *tmp_grounded_src = nullptr;

    for (const auto &platform : platforms) {
        if (check_collision(*platform)) {
            if (platform->get_group_id() == BONUS_TYPE_ID) {
                // ScoreEvent
                udjourney::core::events::ScoreEvent score_event{1};
                m_dispatcher.dispatch(score_event);
                platform->set_state(ActorState::CONSUMED);
            } else if (platform->get_group_id() == MONSTER_TYPE_ID) {
                // Monster collision - damage player (monster attacks)
                using udjourney::Monster;
                Monster *monster = dynamic_cast<Monster *>(platform.get());
                if (monster && !is_invincible()) {
                    // Monster should be in attack state
                    if (monster->is_alive()) {
                        monster->change_state("attack");
                    }

                    // Damage player using health component
                    if (auto *health = get_component<HealthComponent>()) {
                        health->take_damage(1);  // 1 = half heart

                        // Check if player died
                        if (!health->is_alive()) {
                            udj::core::Logger::debug(
                                "Player died! Health: " +
                                std::to_string(health->get_health()));
                            notify("12");  // Game over event
                            return;  // Stop processing - actors vector is being
                                     // modified
                        } else {
                            udj::core::Logger::debug(
                                "Player took damage from monster! Health: " +
                                std::to_string(health->get_health()) + "/" +
                                std::to_string(health->get_max_health()));
                        }
                    }

                    // Grant invincibility after taking damage
                    set_invicibility(2.0f);
                }
                // Skip further processing of this monster
                continue;
            } else if (platform->get_group_id() == PLATFORM_TYPE_ID) {
                Rectangle platformRect = platform->get_rectangle();

                // Use raylib's collision detection
                if (CheckCollisionRecs(r, platformRect)) {
                    Rectangle intersect = GetCollisionRec(r, platformRect);

                    // Determine collision type
                    bool is_vertical = intersect.width > intersect.height;
                    bool from_above = r.y + r.height - intersect.height <
                                      platformRect.y + 1.0f;

                    if (is_vertical && from_above && m_pimpl->velocity_y >= 0) {
                        // Landing on top of platform - snap and ground
                        r.y = platformRect.y - r.height;
                        m_pimpl->velocity_y = 0.0f;
                        tmp_grounded_src =
                            static_cast<Platform *>(platform.get());
                        tmp_grounded = true;
                    } else {
                        // Side or bottom collision - use standard resolution
                        resolve_collision(*platform);
                    }
                    tmp_colliding = true;
                }
            } else {
                resolve_collision(*platform);
                tmp_colliding = true;
            }
        }

        Platform *plat = dynamic_cast<Platform *>(platform.get());

        if (plat) {
            // Platform collision features always apply (even when invincible)
            // Invincibility only prevents damage from monsters, not physics
            for (const auto &feature : plat->get_features()) {
                feature->handle_collision(*plat, *this);
            }
        }
    }
    if (tmp_grounded) {
        // Jump is reset only when the player lands on top of a platform
        _reset_jump();
    }

    // Double-check m_pimpl before accessing (should never be null here)
    if (!m_pimpl) {
        udj::core::Logger::error(
            "Player::handle_collision - m_pimpl became null during collision "
            "processing!");
        return;
    }

    m_pimpl->colliding = tmp_colliding;
    m_pimpl->grounded = tmp_grounded;
    m_pimpl->grounded_src = tmp_grounded_src;
}

// Observable
void Player::add_observer(IObserver *observer) {
    observers.push_back(observer);
}
void Player::remove_observer(IObserver *observer) {
    observers.erase(std::remove(observers.begin(), observers.end(), observer),
                    observers.end());
}
void Player::notify(const std::string &event) {
    for (auto *observer : observers) {
        observer->on_notify(event);
    }
}

void Player::_reset_jump() noexcept {
    if (!m_pimpl) {
        udj::core::Logger::error("Player::_reset_jump - m_pimpl is null!");
        return;
    }
    m_pimpl->jumping = false;
    m_pimpl->current_jumps = 0;  // Reset jump counter on landing
    // Velocity is now managed by gravity system, no manual reset needed
}

void Player::update_animation_state() {
    if (!m_pimpl) {
        udj::core::Logger::error(
            "Player::update_animation_state - m_pimpl is null!");
        return;
    }

    // Check if player is currently moving
    bool is_moving =
        input_mapping.left_pressed() || input_mapping.right_pressed();

    // Get current state before changing
    static PlayerState prev_state = PlayerState::IDLE;

    // Determine player state for animation controller
    PlayerState new_player_state;

    if (m_pimpl->dashing) {
        new_player_state = PlayerState::DASHING;
    } else if (m_pimpl->jumping || !m_pimpl->grounded) {
        new_player_state = PlayerState::JUMPING;
    } else if (is_moving && m_pimpl->grounded) {
        m_is_running = true;
        new_player_state = PlayerState::RUNNING;
    } else {
        m_is_running = false;
        new_player_state = PlayerState::IDLE;
    }

    // Set the current state in the animation controller
    anim_controller_.set_current_state(new_player_state);
}

void Player::attack_nearby_monsters() {
    std::cout << "Player attacking nearby monsters!" << std::endl;
    // Use the event system to notify the game of an attack
    notify("99;attack");  // Custom attack event with mode 99
}

void Player::load_projectile_presets(const std::string &config_file) {
    udj::core::Logger::debug("Loading projectile presets from: " + config_file);
    if (!projectile_loader_) {
        projectile_loader_ =
            std::make_unique<udjourney::ProjectilePresetLoader>();
    }
    bool success = projectile_loader_->load_from_file(config_file);
    udj::core::Logger::debug("Projectile presets loaded: " +
                             std::string(success ? "SUCCESS" : "FAILED"));
}

void Player::set_current_projectile(const std::string &preset_name) {
    if (projectile_loader_ && projectile_loader_->has_preset(preset_name)) {
        if (current_projectile_preset_ == preset_name) {
            return;
        }

        current_projectile_preset_ = preset_name;

        udjourney::core::events::WeaponSelectedEvent weapon_event{
            current_projectile_preset_};
        m_dispatcher.dispatch(weapon_event);
    }
}

void Player::cycle_projectile_type() {
    if (!projectile_loader_) return;

    auto preset_names = projectile_loader_->get_preset_names();
    if (preset_names.empty()) return;

    // Find current preset index
    auto it = std::find(
        preset_names.begin(), preset_names.end(), current_projectile_preset_);

    // Cycle to next preset (wrap around to beginning)
    if (it != preset_names.end()) {
        ++it;
        if (it == preset_names.end()) {
            it = preset_names.begin();
        }
    } else {
        it = preset_names.begin();
    }

    current_projectile_preset_ = *it;
    udj::core::Logger::debug("Switched to projectile: " +
                             current_projectile_preset_);

    udjourney::core::events::WeaponSelectedEvent weapon_event{
        current_projectile_preset_};
    m_dispatcher.dispatch(weapon_event);
}

const udjourney::ProjectilePreset *Player::get_current_projectile_preset()
    const {
    if (!projectile_loader_) return nullptr;
    return projectile_loader_->get_preset(current_projectile_preset_);
}

void Player::reset_shoot_cooldown() {
    shoot_cooldown_ = kShootCooldownDuration;
}

Vector2 Player::get_shoot_position() const {
    // Shoot from outside the player bounds to avoid any collision
    // Use a larger offset to ensure clear separation
    float offset_x = m_facing_right ? r.width + 5.0f : -5.0f;
    return Vector2{r.x + offset_x, r.y + r.height / 2.0f};
}

Vector2 Player::get_shoot_direction() const {
    return Vector2{m_facing_right ? 1.0f : -1.0f, 0.0f};
}

}  // namespace udjourney
