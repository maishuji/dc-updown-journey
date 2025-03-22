// Copyright 2025 Quentin Cartier

#include "udjourney/Player.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include "raylib/raymath.h"
#include "raylib/rlgl.h"

std::vector<IObserver *> observers;

Player::Player(const IGame &game, Rectangle r) : IActor(game), r(r) {}

void Player::draw() const { DrawRectangleRec(r, RED); }

void Player::update(float delta) {
    // Gravity
    r.y += 1;
    if(r.y > get_game().get_rectangle().height) {
        r.y = 0;
        notify("12"); // Game over
    }
}

void Player::process_input(cont_state_t *cont) {
    if (cont->buttons & CONT_DPAD_LEFT) {
        r.x -= 5;
    }
    if (cont->buttons & CONT_DPAD_RIGHT) {
        r.x += 5;
    }
    if (cont->buttons & CONT_DPAD_UP) {
        r.y -= 5;
    }
    if (cont->buttons & CONT_DPAD_DOWN) {
        r.y += 5;
    }
}

void Player::resolve_collision(const IActor &platform) noexcept {
    auto platformRect = platform.get_rectangle();
    if (CheckCollisionRecs(r, platformRect)) {
        float overlapLeft = r.x + r.width - platformRect.x;
        float overlapRight = platformRect.x + platformRect.width - r.x;
        float overlapTop = r.y + r.height - platformRect.y;
        float overlapBottom = platformRect.y + platformRect.height - r.y;

        // Resolve the smallest overlap (to avoid diagonal teleportation)
        if (overlapLeft < overlapRight && overlapLeft < overlapTop &&
            overlapLeft < overlapBottom) {
            r.x = platformRect.x - r.width;  // Adjust left
        } else if (overlapRight < overlapLeft && overlapRight < overlapTop &&
                   overlapRight < overlapBottom) {
            r.x = platformRect.x + platformRect.width;  // Adjust right
        } else if (overlapTop < overlapLeft && overlapTop < overlapRight &&
                   overlapTop < overlapBottom) {
            r.y = platformRect.y - r.height;  // Adjust top
        } else {
            r.y = platformRect.y + platformRect.height;  // Adjust bottom
        }
    }
}

void Player::handle_collision(
    const std::vector<std::unique_ptr<IActor>> &platforms) noexcept {
    const uint8_t BONUS_TYPE_ID = 2;
    for (const auto &platform : platforms) {
        if (check_collision(*platform)) {
            if (platform->get_group_id() == BONUS_TYPE_ID) notify("1;1");
            resolve_collision(*platform);
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
