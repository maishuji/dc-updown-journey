// Copyright 2025 Quentin Cartier

#include "udjourney/Player.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rlgl.h"

std::vector<IObserver *> observers;

Player::Player(const IGame &game, Rectangle r) : IActor(game), r(r) {}

void Player::draw() const {
    auto rect = r;
    auto &game = get_game();
    // Convert to screen coordinates
    rect.x -= game.get_rectangle().x;
    rect.y -= game.get_rectangle().y;
    DrawRectangleRec(rect, m_colliding ? RED : GREEN);
}

void Player::update(float delta) {
    // Gravity
    r.y += 1;

    const auto &gameRect = get_game().get_rectangle();

    // Gameover it the player is out of the screen at the bottom
    if (r.y > gameRect.y + gameRect.height) {
        r.y = gameRect.y + r.height;
        notify("12");  // Game over
    }
}

void Player::process_input(cont_state_t *cont) {
    if (cont->buttons & CONT_DPAD_LEFT) {
        r.x -= 5;
    }
    if (cont->buttons & CONT_DPAD_RIGHT) {
        r.x += 5;
    }
    // if (cont->buttons & CONT_DPAD_UP) {
    //     r.y -= 5;
    // }
    if (cont->buttons & CONT_DPAD_DOWN) {
        r.y += 5;
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

void Player::handle_collision(
    const std::vector<std::unique_ptr<IActor>> &platforms) noexcept {
    const auto &gameRect = get_game().get_rectangle();

    // Dont check collision if the player is out of the screen at the top
    if (this->get_rectangle().y < gameRect.y) {
        return;
    }

    const uint8_t BONUS_TYPE_ID = 2;
    m_colliding = false;
    for (const auto &platform : platforms) {
        if (check_collision(*platform)) {
            if (platform->get_group_id() == BONUS_TYPE_ID) {
                notify("1;1");
                platform->set_state(ActorState::CONSUMED);
            }
            resolve_collision(*platform);
            m_colliding = true;
        }
    }
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
