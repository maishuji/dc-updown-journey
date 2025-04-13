// Copyright 2025 Quentin Cartier
#include "udjourney/Game.hpp"

#include <kos.h>
#include <raylib/raymath.h>
#include <raylib/rlgl.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "udjourney/Bonus.hpp"
#include "udjourney/IActor.hpp"
#include "udjourney/Platform.hpp"
#include "udjourney/Player.hpp"

const double kUpdateInterval = 0.0001;
bool is_running = true;
std::unique_ptr<Player> player = nullptr;
int64_t score = 0;

std::vector<std::unique_ptr<IActor>> init_platforms(const Game &game) {
    std::vector<std::unique_ptr<IActor>> res;
    int lastx = 0;
    int lastx2 = 100;

    for (int i = 0; i < 800; i += 100) {
        auto r = Rectangle(lastx, i, lastx2, 5);
        res.emplace_back(std::make_unique<Platform>(game, r));
        int ra = std::rand();
        lastx = (ra % 10) * 50;
        lastx2 = ra % 100 + 50;
    }
    return res;
}

Game::Game(int w, int h) : IGame(), m_state(GameState::TITLE) {
    r = Rectangle(0, 0, w, h);
    m_actors.reserve(10);
}

void Game::run() {
    m_actors = init_platforms(*this);
    player = std::make_unique<Player>(*this, Rectangle(320, 240, 20, 20));
    player->add_observer(static_cast<IObserver *>(this));
    // m_actors.push_back(std::move(player));
    //  m_actors.emplace_back(std::make_unique<Player>(*this, Rectangle(320,
    //  240, 20, 20)));
    m_actors.emplace_back(
        std::make_unique<Bonus>(*this, Rectangle(300, 300, 20, 20)));

    InitWindow(r.width, r.height, "Up-Down Journey");
    SetTargetFPS(60);
    last_update_time = GetTime();
    m_state = GameState::PLAY;

    while (is_running) {
        update();
    }
}

void Game::add_actor(std::unique_ptr<IActor> actor) {
    if (m_updating_actors) {
        m_pending_actors.push_back(std::move(actor));
    } else {
        m_actors.push_back(std::move(actor));
    }
}

void Game::remove_actor(IActor *actor) {
    if (actor == nullptr) return;
    auto iter = std::find_if(m_actors.begin(),
                             m_actors.end(),
                             [&actor](const std::unique_ptr<IActor> &p) {
                                 return p.get() == actor;
                             });
    if (iter != m_actors.end()) {
        m_actors.erase(iter);
    }
}

void Game::process_input(cont_state_t *cont) {
    if (IsGamepadAvailable(0)) {
        // Press 'B' to quit
        bool bPressed = IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
        if (bPressed) {
            // Press b to quit
            is_running = false;
        }
    }

    for (auto &a : m_actors) {
        a->process_input(cont);
    }
    player->process_input(cont);
}

void Game::draw() const {
    BeginDrawing();
    ClearBackground(BLACK);  // Clear the background with a color

    // Draw the rectangle
    // DrawRectangleRec(r, BLUE);
    DrawText(std::to_string(score).c_str(), 10, 30, 20, BLUE);
    if (m_state == GameState::PLAY) {
        for (const auto &p : m_actors) {
            p->draw();
        }
        player->draw();
    } else if (m_state == GameState::PAUSE) {
        DrawText(" -- PAUSE -- \n", 10, 10, 20, RED);
    } else {
        DrawText("Hello, World. Press START to break.\n", 10, 10, 20, RED);
    }
    DrawText("Press B button to quit\n", 300, 40, 20, RED);

    DrawFPS(10, 50);  // Draw FPS counter

    EndDrawing();
}

void Game::update() {
    // Update actors
    if (m_state == GameState::PLAY) {
        m_updating_actors = true;
        for (auto &p : m_actors) {
            p->update(0.0f);
        }
        player->update(0.0f);

        m_updating_actors = false;

        // Move pending actors to actors
        for (auto &pa : m_pending_actors) {
            m_actors.push_back(std::move(pa));
        }
        m_pending_actors.clear();
    }  // GameState::PLAY

    // Removing CONSUMED actors (DEAD and ready for removing)
    for (auto &p : m_actors) {
        if (p->get_state() == ActorState::CONSUMED) {
            remove_actor(p.get());
        }
    }

    maple_device_t *controller = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
    if (controller) {
        cont_state_t *cont =
            reinterpret_cast<cont_state_t *>(maple_dev_status(controller));
        if (cont) {
            // Preprocessing of input
            auto startPressed =
                IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT);
            if (startPressed) {
                if (m_state == GameState::PLAY)
                    m_state = GameState::PAUSE;
                else
                    m_state = GameState::PLAY;
            }
            process_input(cont);
        }
    }

    double cur_update_time = GetTime();
    if (cur_update_time - last_update_time > kUpdateInterval) {
        r.y += 1;

        for (auto &p : m_actors) {
            p->update(0.0f);
        }
        player->update(0.0f);
        last_update_time = cur_update_time;
    }

    player->handle_collision(m_actors);

    draw();
}

// Function definition for extract_number_
std::optional<int16_t> extract_number_(const std::string_view &s) {
    std::string number;
    for (char c : s) {
        if (std::isdigit(c)) {
            number += c;
        }
    }
    try {
        return std::stoi(number);  // Convert each token to integer
    } catch (const std::invalid_argument &e) {
        std::cerr << "Invalid number: " << number << std::endl;
    } catch (const std::out_of_range &e) {
        std::cerr << "Number out of range: " << number << std::endl;
    }
    return {};
}

void Game::on_notify(const std::string &event) {
    std::stringstream ss(event);
    std::string token;
    uint8_t line_idx = 0;
    int mode = 0;
    while (std::getline(ss, token, ';')) {  // Split by ';'
        if (line_idx == 0) {
            auto mode_opt = extract_number_(token);
            if (mode_opt.has_value()) {
                mode = mode_opt.value();
                if (mode == 12) {
                    // Game Over event
                    m_state = GameState::GAMEOVER;
                }
            } else {
                // Invalid mode, nothing to do
                break;
            }

        } else {
            switch (mode) {
                case 1: {
                    // Parsing scoring event
                    std::optional<int16_t> score_inc_opt =
                        extract_number_(token);
                    if (score_inc_opt.has_value()) {
                        score += score_inc_opt.value();
                    }
                } break;

                default:
                    break;
            }
        }
        line_idx++;
    }

    int v = std::stoi(event);
    score += v;
}
