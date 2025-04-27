// Copyright 2025 Quentin Cartier
#include "udjourney/Game.hpp"

#include <kos.h>
#include <raylib/raymath.h>
#include <raylib/rlgl.h>

#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "udjourney/Bonus.hpp"
#include "udjourney/IActor.hpp"
#include "udjourney/Player.hpp"
#include "udjourney/ScoreHistory.hpp"
#include "udjourney/platform/Platform.hpp"
#include "udjourney/platform/reuse_strategies/PlatformReuseStrategy.hpp"
#include "udjourney/platform/reuse_strategies/RandomizePositionStrategy.hpp"

enum class ActorType : uint8_t {
    PLAYER = 0,
    PLATFORM = 1,
    BONUS = 2,
};

const double kUpdateInterval = 0.0001;
bool is_running = true;
std::unique_ptr<Player> player = nullptr;
std::unique_ptr<PlatformReuseStrategy> reuse_strategy =
    std::make_unique<RandomizePositionStrategy>();

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

    // Create borders
    const int border_width = 5;
    const int border_height = 100;
    auto game_rect = game.get_rectangle();

    constexpr int cpool_size = 5;
    std::array<Color, cpool_size> color_pool = {
        GRAY, PINK, BROWN, YELLOW, PURPLE};

    int color_idx = 0;
    // The left and right borders
    // Note that they are repeated when running out of the screen
    for (auto border_top = 0; border_top <= game_rect.height + border_height;
         border_top += border_height) {
        res.emplace_back(std::make_unique<Platform>(
            game,
            Rectangle(0, border_top, border_width, border_height),
            color_pool[color_idx],
            true));
        res.emplace_back(
            std::make_unique<Platform>(game,
                                       Rectangle(game_rect.width - border_width,
                                                 border_top,
                                                 border_width,
                                                 border_height),
                                       color_pool[color_idx],
                                       true));
        color_idx = (color_idx + 1) % cpool_size;
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

    bonus_manager.add_observer(static_cast<IObserver *>(this));

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

void _draw_title() {
    DrawText(" -- UP-DOWN JOURNEY -- \n", 10, 10, 20, RED);
    DrawText("Press START to start the game\n", 300, 40, 20, RED);
    DrawText("Press B button to quit\n", 300, 70, 20, RED);
}

void _draw_game_over(auto score_history) {
    DrawText("GAME OVER - Press START to start over.\n", 10, 10, 20, RED);
    DrawText("Press B button to quit\n", 300, 40, 20, RED);

    const int kScoreX = 300;
    const int kScoreOffsetX =
        50;  // Offset for the score conpared to the header
    const int kScoreStartY = 70;
    const int kScoreSize = 20;
    const int kScoreYStep = kScoreSize + 2;
    const int kScoreMaxLines = 10;

    std::stringstream ss;
    DrawText("Scores: ", kScoreX, kScoreStartY, kScoreSize, YELLOW);
    int idx = 1;
    auto scores = score_history.get_scores();
    for (auto it = scores.rbegin(); it != scores.rend(); ++it) {
        auto s = std::to_string(*it);
        DrawText(s.c_str(),
                 kScoreX + kScoreOffsetX,
                 kScoreStartY + idx * kScoreYStep,
                 kScoreSize,
                 WHITE);
        ++idx;
        if (idx > kScoreMaxLines) {
            break;  // Limit to 10 scores
        }
    }
}

void _draw_pause() {
    DrawText(" -- PAUSE -- \n", 10, 10, 20, RED);
    DrawText("Press B button to quit\n", 300, 40, 20, RED);
}

void Game::draw() const {
    BeginDrawing();
    ClearBackground(BLACK);  // Clear the background with a color

    // Draw the rectangle
    // DrawRectangleRec(r, BLUE);
    std::stringstream ss;

    switch (m_state) {
        case GameState::TITLE:
            _draw_title();
            break;
        case GameState::PLAY: {
            for (const auto &p : m_actors) {
                p->draw();
            }
            player->draw();
        }
            DrawFPS(10, 50);  // Draw FPS counter
            ss << "Score: ";
            ss << std::to_string(m_score);
            DrawText(ss.str().c_str(), 10, 30, 20, BLUE);  // Draw current score
            break;
        case GameState::PAUSE:
            _draw_pause();
            break;
        case GameState::GAMEOVER:
            _draw_game_over(m_score_history);
            break;
    }
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
            switch (p->get_group_id()) {
                {
                    case static_cast<uint8_t>(ActorType::PLATFORM): {
                        // Instead of removing the platform, we can reuse it
                        reuse_strategy->reuse(static_cast<Platform &>(*p));
                        /*
                         */
                    } break;
                    case static_cast<uint8_t>(ActorType::BONUS): {
                        remove_actor(p.get());
                    } break;
                    default:
                        break;
                }
            }
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
    float delta = static_cast<float>(cur_update_time - last_update_time);
    bonus_manager.update(delta);
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

void _process_bonus(IGame &game, std::stringstream &token_stream) {
    std::string str_v1, str_v2;
    int16_t v1 = 0;
    int16_t v2 = 0;

    std::getline(token_stream, str_v1, '+');
    std::getline(token_stream, str_v2, '+');

    // Currently numbers extracted are bounded [0-100)
    if (std::optional<int16_t> v1_opt = extract_number_(str_v1);
        v1_opt.has_value()) {
        v1 = v1_opt.value();
    }
    if (std::optional<int16_t> v2_opt = extract_number_(str_v2);
        v2_opt.has_value()) {
        v2 = v2_opt.value();
    }

    const auto kRectSize = 20;

    auto x = game.get_rectangle().x +
             ((game.get_rectangle().width - kRectSize) / 100.0) * v1;
    auto y = game.get_rectangle().y + game.get_rectangle().height / 2.0f +
             (game.get_rectangle().height / 200.0) * v2;

    auto bonus =
        std::make_unique<Bonus>(game, Rectangle(x, y, kRectSize, kRectSize));
    game.add_actor(std::move(bonus));
}

void Game::on_notify(const std::string &event) {
    std::stringstream ss(event);
    std::string token;
    int mode = 0;

    const int16_t kModeGameOuver = 12;
    const int16_t kModeScoring = 1;
    const int16_t kModeBonus = 2;

    // Parse event mode
    if (std::getline(ss, token, ';')) {
        // First token is the mode
        auto mode_opt = extract_number_(token);
        if (mode_opt.has_value()) {
            mode = mode_opt.value();
        } else {
            std::cerr << "Invalid mode: " << token << std::endl;
            return;
        }
    }  // Split by ';'

    std::cout << "Event: " << ss.str() << std::endl;
    switch (mode) {
        case kModeGameOuver:
            m_state = GameState::GAMEOVER;
            std::cout << "Game Over" << std::endl;
            m_score_history.add_score(m_score);
            m_score = 0;  // Reset score
            break;
        case kModeScoring:
            // Parsing scoring event
            if (std::optional<int16_t> score_inc_opt = extract_number_(token);
                score_inc_opt.has_value()) {
                m_score += score_inc_opt.value();
            }
            break;
        case kModeBonus:
            // Parsing bonus event
            _process_bonus(*this, ss);

            break;
        default:
            break;
    }
}
