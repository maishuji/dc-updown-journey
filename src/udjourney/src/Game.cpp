// Copyright 2025 Quentin Cartier
#include "udjourney/Game.hpp"
#ifdef PLATFORM_DREAMCAST
#include <kos.h>
#endif
#include <raylib/raymath.h>
#include <raylib/rlgl.h>

#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "udjourney/Bonus.hpp"
#include "udjourney/Player.hpp"
#include "udjourney/ScoreHistory.hpp"
#include "udjourney/hud/HUDComponent.hpp"
#include "udjourney/hud/ScoreHUD.hpp"
#include "udjourney/interfaces/IActor.hpp"
#include "udjourney/platform/Platform.hpp"
#include "udjourney/platform/behavior_strategies/HorizontalBehaviorStrategy.hpp"
#include "udjourney/platform/behavior_strategies/OscillatingSizeBehaviorStrategy.hpp"
#include "udjourney/platform/reuse_strategies/PlatformReuseStrategy.hpp"
#include "udjourney/platform/reuse_strategies/RandomizePositionStrategy.hpp"

enum class ActorType : uint8_t {
    PLAYER = 0,
    PLATFORM = 1,
    BONUS = 2,
};

namespace {
struct InputMapping {
    std::function<bool()> pressed_start;
    std::function<bool()> pressed_B;
    InputMapping() {
#ifdef PLATFORM_DREAMCAST
        pressed_start = []() {
            return IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT);
        };
        pressed_B = []() {
            return IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
        };
#else
        pressed_start = []() { return IsKeyPressed(KEY_ENTER); };
        pressed_B = []() { return IsKeyPressed(KEY_P); };
#endif
    }
} input_mapping;
}  // namespace

const double kUpdateInterval = 0.0001;
bool is_running = true;
std::unique_ptr<Player> player = nullptr;
std::unique_ptr<PlatformReuseStrategy> reuse_strategy =
    std::make_unique<RandomizePositionStrategy>();

struct DashFud {
    int16_t dashable = 1;
} dash_fud;

std::vector<std::unique_ptr<IActor>> init_platforms(const Game &iGame) {
    std::vector<std::unique_ptr<IActor>> res;
    int lastx = 0;
    int lastx2 = 100;

    const int kOffsetPosXMin = 50;
    const int kMaxWidth = 100;

    const int kStepY = 100;
    const int kMaxY = 800;

    for (int cur_pos_y = 0; cur_pos_y < kMaxY; cur_pos_y += kStepY) {
        Rectangle rect{static_cast<float>(lastx),
                       static_cast<float>(cur_pos_y),
                       static_cast<float>(lastx2),
                       5};
        res.emplace_back(std::make_unique<Platform>(iGame, rect));
        int random_number = std::rand();
        lastx = (random_number % 10) * kOffsetPosXMin;
        lastx2 = random_number % kMaxWidth + kOffsetPosXMin;

        auto ra2 = std::rand();
        if (ra2 % 100 < 20) {
            float speed_x = static_cast<float>(std::max(5, random_number % 30));
            float max_offset = static_cast<float>(
                std::max(kOffsetPosXMin, random_number % kMaxWidth));

            // 20% of moving platforms
            static_cast<Platform *>(res.back().get())
                ->set_behavior(std::make_unique<HorizontalBehaviorStrategy>(
                    speed_x, max_offset));
        } else if (ra2 % 100 < 40) {
            float speed_x = static_cast<float>(std::max(5, random_number % 30));

            const int kShrinkMinOffset = -100;
            const int kShrinkMaxOffset = 150;

            int min_offset =
                kShrinkMinOffset +
                (std::rand() % (1 + std::abs(kShrinkMinOffset)));   // -100 to 0
            int max_offset = std::rand() % (kShrinkMaxOffset + 1);  // 0 to 150

            static_cast<Platform *>(res.back().get())
                ->set_behavior(
                    std::make_unique<OscillatingSizeBehaviorStrategy>(
                        speed_x,
                        static_cast<float>(min_offset),
                        static_cast<float>(max_offset)));
        }
    }

    // Create borders
    const int border_width = 5;
    const int border_height = 100;
    auto game_rect = iGame.get_rectangle();

    constexpr int cpool_size = 5;
    std::array<Color, cpool_size> color_pool = {
        GRAY, PINK, BROWN, YELLOW, PURPLE};

    int color_idx = 0;
    // The left and right borders
    // Note that they are repeated when running out of the screen
    for (auto border_top = 0;
         border_top <= static_cast<int>(game_rect.height + border_height);
         border_top += border_height) {
        res.emplace_back(std::make_unique<Platform>(
            iGame,
            Rectangle{0,
                      static_cast<float>(border_top),
                      static_cast<float>(border_width),
                      static_cast<float>(border_height)},
            color_pool[color_idx],
            true));
        res.emplace_back(std::make_unique<Platform>(
            iGame,
            Rectangle{game_rect.width - border_width,
                      static_cast<float>(border_top),
                      static_cast<float>(border_width),
                      static_cast<float>(border_height)},
            color_pool[color_idx],
            true));
        color_idx = (color_idx + 1) % cpool_size;
    }

    return res;
}

Game::Game(int iWidth, int iHeight) : IGame() {
    m_rect = Rectangle{
        0, 0, static_cast<float>(iWidth), static_cast<float>(iHeight)};
    m_actors.reserve(10);
}

void Game::run() {
    InitWindow(static_cast<int>(m_rect.width),
               static_cast<int>(m_rect.height),
               "Up-Down Journey");

    m_actors = init_platforms(*this);
    player = std::make_unique<Player>(*this, Rectangle{320, 240, 20, 20});
    player->add_observer(static_cast<IObserver *>(this));

    m_actors.emplace_back(
        std::make_unique<Bonus>(*this, Rectangle{300, 300, 20, 20}));

    SetTargetFPS(60);
    m_last_update_time = GetTime();
    m_state = GameState::PLAY;

    m_bonus_manager.add_observer(static_cast<IObserver *>(this));

    auto score_hud = std::make_unique<ScoreHUD>(Vector2{10, 50}, player.get());
    m_hud_manager.add(std::move(score_hud));

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
    if (actor != nullptr) {
        auto iter =
            std::find_if(m_actors.begin(),
                         m_actors.end(),
                         [&actor](const std::unique_ptr<IActor> &actor2) {
                             return actor2.get() == actor;
                         });
        if (iter != m_actors.end()) {
            m_actors.erase(iter);
        }
    }
}

void Game::process_input() {
    if (WindowShouldClose()) {
        is_running = false;
        return;
    }
    // Press 'B' to quit
    bool bPressed = input_mapping.pressed_B();
    if (bPressed) {
        // Press b to quit
        is_running = false;
    }

    // Pause / Unpause the game
    auto startPressed = input_mapping.pressed_start();
    if (startPressed) {
        if (m_state == GameState::PLAY) {
            m_state = GameState::PAUSE;
        } else {
            m_state = GameState::PLAY;
        }
    }
    if (m_state == GameState::PLAY) {
        for (auto &actor : m_actors) {
            actor->process_input();
        }
        player->process_input();
    }
}

void draw_title_() {
    DrawText(" -- UP-DOWN JOURNEY -- \n", 10, 10, 20, RED);
    DrawText("Press START to start the game\n", 300, 40, 20, RED);
    DrawText("Press B button to quit\n", 300, 70, 20, RED);
}

void draw_game_over_(auto score_history) {
    DrawText("GAME OVER - Press START to start over.\n", 10, 10, 20, RED);
    DrawText("Press B button to quit\n", 300, 40, 20, RED);

    const int kScoreX = 300;
    const int kScoreOffsetX =
        50;  // Offset for the score conpared to the header
    const int kScoreStartY = 70;
    const int kScoreSize = 20;
    const int kScoreYStep = kScoreSize + 2;
    const int kScoreMaxLines = 10;

    std::stringstream str_stream;
    DrawText("Scores: ", kScoreX, kScoreStartY, kScoreSize, YELLOW);
    int idx = 1;
    auto scores = score_history.get_scores();
    for (auto iter = scores.rbegin(); iter != scores.rend(); ++iter) {
        auto str_token = std::to_string(*iter);
        DrawText(str_token.c_str(),
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

void draw_pause_() {
    DrawText(" -- PAUSE -- \n", 10, 10, 20, RED);
    DrawText("Press B button to quit\n", 300, 40, 20, RED);
}

void Game::draw() const {
    BeginDrawing();
    ClearBackground(BLACK);  // Clear the background with a color

    // Draw the rectangle
    std::stringstream str_stream;

    switch (m_state) {
        case GameState::TITLE:
            draw_title_();
            break;
        case GameState::PLAY: {
            for (const auto &actor : m_actors) {
                actor->draw();
            }
            player->draw();
        }

            // Draw dash status
            DrawCircle(static_cast<int>(get_rectangle().width) - 50,
                       45,
                       17,
                       dash_fud.dashable == 1 ? GREEN : RED);

            break;
        case GameState::PAUSE:
            draw_pause_();
            break;
        case GameState::GAMEOVER:
            draw_game_over_(m_score_history);
            break;
    }
    m_hud_manager.draw();
    EndDrawing();
}

void Game::update() {
    static double last_update_time = 0.0;

    // Update actors
    if (m_state == GameState::PLAY) {
        m_updating_actors = true;
        for (auto &actor : m_actors) {
            actor->update(0.0F);
        }
        player->update(0.0F);

        m_updating_actors = false;

        // Move pending actors to actors
        for (auto &pending_actor : m_pending_actors) {
            m_actors.push_back(std::move(pending_actor));
        }
        m_pending_actors.clear();
    }  // GameState::PLAY

    // Removing CONSUMED actors (DEAD and ready for removing)
    std::vector<IActor *> to_remove;  // Gathering actors to remove
    for (auto &actor : m_actors) {
        if (actor->get_state() == ActorState::CONSUMED) {
            switch (actor->get_group_id()) {
                {
                    case static_cast<uint8_t>(ActorType::PLATFORM): {
                        // Instead of removing the platform, we can reuse it
                        reuse_strategy->reuse(static_cast<Platform &>(*actor));
                        /*
                         */
                    } break;
                    case static_cast<uint8_t>(ActorType::BONUS): {
                        to_remove.push_back(actor.get());
                    } break;
                    default:
                        break;
                }
            }
        }
    }
    if (!to_remove.empty()) {
        for (auto *actor : to_remove) {
            remove_actor(actor);
        }
    }

    process_input();

    double cur_update_time = GetTime();
    auto delta = static_cast<float>(cur_update_time - last_update_time);
    m_bonus_manager.update(delta);
    if (cur_update_time - last_update_time > kUpdateInterval) {
        m_rect.y += 1;
        for (auto &actor : m_actors) {
            actor->update(delta);
        }
        player->update(delta);
        last_update_time = cur_update_time;
    }

    player->handle_collision(m_actors);

    m_hud_manager.update(delta);

    draw();
}

// Function definition for extract_number_
std::optional<int16_t> extract_number_(const std::string_view &iStrView) {
    std::string number;
    for (char letter : iStrView) {
        auto is_digit = std::isdigit(letter);
        if (is_digit != 0) {
            number += letter;
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

void process_bonus_(IGame &game, std::stringstream &token_stream) {
    std::string str_v1;
    std::string str_v2;
    int16_t value_1 = 0;
    int16_t value_2 = 0;

    std::getline(token_stream, str_v1, '+');
    std::getline(token_stream, str_v2, '+');

    // Currently numbers extracted are bounded [0-100)
    if (std::optional<int16_t> v1_opt = extract_number_(str_v1);
        v1_opt.has_value()) {
        value_1 = v1_opt.value();
    }
    if (std::optional<int16_t> v2_opt = extract_number_(str_v2);
        v2_opt.has_value()) {
        value_2 = v2_opt.value();
    }

    const auto kRectSize = 20;

    auto pos_x = game.get_rectangle().x +
                 ((game.get_rectangle().width - kRectSize) / 100.0) * value_1;
    auto pos_y = game.get_rectangle().y + game.get_rectangle().height / 2.0F +
                 (game.get_rectangle().height / 200.0) * value_2;

    auto bonus =
        std::make_unique<Bonus>(game,
                                Rectangle{static_cast<float>(pos_x),
                                          static_cast<float>(pos_y),
                                          static_cast<float>(kRectSize),
                                          static_cast<float>(kRectSize)});
    game.add_actor(std::move(bonus));
}

void Game::on_notify(const std::string &iEvent) {
    std::stringstream str_stream(iEvent);
    std::string token;
    int mode = 0;

    const int16_t kModeGameOuver = 12;
    const int16_t kModeScoring = 1;
    const int16_t kModeBonus = 2;
    const int16_t kModeDash = 4;

    // Parse event mode
    if (std::getline(str_stream, token, ';')) {
        // First token is the mode
        auto mode_opt = extract_number_(token);
        if (mode_opt.has_value()) {
            mode = mode_opt.value();
        } else {
            std::cerr << "Invalid mode: " << token << std::endl;
            return;
        }
    }  // Split by ';'

    std::cout << "Event: " << str_stream.str() << std::endl;
    switch (mode) {
        case kModeGameOuver:
            m_state = GameState::GAMEOVER;
            std::cout << "Game Over" << std::endl;
            m_score_history.add_score(m_score);
            m_score = 0;  // Reset score
            break;
        case kModeScoring:
            // Parsing scoring event
            std::getline(str_stream, token, ';');
            if (std::optional<int16_t> score_inc_opt = extract_number_(token);
                score_inc_opt.has_value()) {
                m_score += score_inc_opt.value();
            }
            break;
        case kModeBonus:
            // Parsing bonus event
            process_bonus_(*this, str_stream);
            break;
        case kModeDash:

            // Parsing dash event
            std::getline(str_stream, token, ';');
            std::cout << " dash event : " << token << std::endl;
            if (std::optional<int16_t> dash_opt = extract_number_(token);
                dash_opt.has_value()) {
                dash_fud.dashable = dash_opt.value();
            }
            break;
        default:
            break;
    }
}
