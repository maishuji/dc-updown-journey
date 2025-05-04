// Copyright 2025 Quentin Cartier

#include "udjourney/Player.hpp"

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

#include "raylib/raymath.h"
#include "raylib/rlgl.h"
#include "udjourney/CoreUtils.hpp"

Player::~Player() = default;

namespace {
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
        left_pressed = []() { return IsKeyDown(KEY_LEFT); };
        right_pressed = []() { return IsKeyDown(KEY_RIGHT); };
        up_pressed = []() { return IsKeyDown(KEY_UP); };
        down_pressed = []() { return IsKeyDown(KEY_DOWN); };
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
    float vy = 0.0f;
    float dash_timer = 0.0f;
    float dash_cooldown = 0.0f;
};

std::vector<IObserver *> observers;

Player::Player(const IGame &game, Rectangle r) :
    IActor(game), r(r), m_pimpl(std::make_unique<Player::PImpl>()) {}

void Player::draw() const {
    auto rect = r;
    auto &game = get_game();
    // Convert to screen coordinates
    rect.x -= game.get_rectangle().x;
    rect.y -= game.get_rectangle().y;
    DrawRectangleRec(rect,
                     m_pimpl->grounded    ? BLUE
                     : m_pimpl->colliding ? RED
                     : m_pimpl->dashing ? ORANGE
                                          : GREEN);
}

void Player::update(float delta) {
    // Gravity
    r.y += 1;
    if (m_pimpl->jumping) {
        using udjourney::coreutils::math::is_near_zero;
        using udjourney::coreutils::math::is_same_sign;
        r.y += m_pimpl->vy;
        float old_vy = m_pimpl->vy;
        m_pimpl->vy += 0.1f;  // exhaustion
        if (!is_same_sign(old_vy, m_pimpl->vy) || is_near_zero(m_pimpl->vy)) {
            _reset_jump();
        }
    }

    // Dash timers
    if (m_pimpl->dashing) {
        m_pimpl->dash_timer -= delta;
        if (m_pimpl->dash_timer <= 0.0f) {
            m_pimpl->dashing = false;
        }
    }
    if (!m_pimpl->dashing && !m_pimpl->dashable) {
        if (m_pimpl->dash_cooldown > 0.0f) {
            m_pimpl->dash_cooldown -= delta;
        } else {
            m_pimpl->dashable = true;
            notify("4;1");
        }
    }

    const auto &gameRect = get_game().get_rectangle();

    // Gameover it the player is out of the screen at the bottom
    if (r.y > gameRect.y + gameRect.height) {
        r.y = gameRect.y + r.height;
        notify("12");  // Game over
    }
}

void Player::process_input() {
    if (input_mapping.left_pressed()) {
        r.x -= m_pimpl->dashing ? 12 : 5;
    }
    if (input_mapping.right_pressed()) {
        r.x += m_pimpl->dashing ? 12 : 5;
    }

    // A to Jump
    if (input_mapping.jump_pressed()) {
        if (!m_pimpl->jumping && m_pimpl->grounded) {
            m_pimpl->jumping = true;
            r.y -= 5;
            m_pimpl->vy = -5.0f;
        }
    } else {
        m_pimpl->jumping = false;
    }

    // DPAD down
    if (input_mapping.down_pressed()) {
        r.y += 5;
    }

    // Dash input
    if ((input_mapping.dash_pressed()) && m_pimpl->dash_cooldown <= 0.0f) {
        m_pimpl->dashing = true;
        m_pimpl->dash_timer = 0.2f;     // Dash lasts 0.2 seconds
        m_pimpl->dash_cooldown = 1.0f;  // Then 1 second cooldown
        m_pimpl->dashable =
            false;  // Player can't dash again until cooldown is over
        notify("4;0");
    }
}

void Player::resolve_collision(const IActor &platform) noexcept {
    auto platformRect = platform.get_rectangle();
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
 * @param platforms A vector of unique pointers to IActor objects representing
 * the platforms
 * @return void
 * @throws none
 */
void Player::handle_collision(
    const std::vector<std::unique_ptr<IActor>> &platforms) noexcept {
    const auto &gameRect = get_game().get_rectangle();

    // Dont check collision if the player is out of the screen at the top
    if (this->get_rectangle().y < gameRect.y) {
        return;
    }

    const uint8_t PLATFORM_TYPE_ID = 1;
    const uint8_t BONUS_TYPE_ID = 2;

    bool tmp_colliding = false;
    bool tmp_grounded = false;

    for (const auto &platform : platforms) {
        if (check_collision(*platform)) {
            if (platform->get_group_id() == BONUS_TYPE_ID) {
                notify("1;1");
                platform->set_state(ActorState::CONSUMED);
            } else if (platform->get_group_id() == PLATFORM_TYPE_ID) {
                // Check grounded
                if (r.y < platform->get_rectangle().y) {
                    tmp_grounded = true;
                }
            }
            resolve_collision(*platform);
            tmp_colliding = true;
        }
    }
    if (tmp_colliding) {
        // Jump is reset as soon as the player collides with a platform
        _reset_jump();
    }
    m_pimpl->colliding = tmp_colliding;
    m_pimpl->grounded = tmp_grounded;
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
    m_pimpl->jumping = false;
    m_pimpl->vy = 0.0f;
}
