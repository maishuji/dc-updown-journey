// Copyright 2025 Quentin Cartier
#include "udjourney/Game.hpp"
#ifdef PLATFORM_DREAMCAST
#include <kos.h>
#endif
#include <raylib/raymath.h>
#include <raylib/rlgl.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "udjourney/Bonus.hpp"
#include "udjourney/CoreUtils.hpp"
#include "udjourney/Monster.hpp"
#include "udjourney/Player.hpp"
#include "udjourney/AnimSpriteController.hpp"
#include "udjourney/SpriteAnim.hpp"
#include "udjourney/ScoreHistory.hpp"
#include "udjourney/core/Logger.hpp"
#include "udjourney/hud/DialogBoxHUD.hpp"
#include "udjourney/hud/HUDComponent.hpp"
#include "udjourney/hud/LevelSelectHUD.hpp"
#include "udjourney/hud/ScoreHUD.hpp"
#include "udjourney/interfaces/IActor.hpp"
#include "udjourney/managers/TextureManager.hpp"
#include "udjourney/loaders/AnimationConfigLoader.hpp"
#include "udjourney/platform/Platform.hpp"
#include "udjourney/platform/behavior_strategies/EightTurnHorizontalBehaviorStrategy.hpp"
#include "udjourney/platform/behavior_strategies/HorizontalBehaviorStrategy.hpp"
#include "udjourney/platform/behavior_strategies/OscillatingSizeBehaviorStrategy.hpp"
#include "udjourney/platform/features/CheckpointFeature.hpp"
#include "udjourney/platform/features/PlatformFeatureBase.hpp"
#include "udjourney/platform/features/SpikeFeature.hpp"
#include "udjourney/platform/reuse_strategies/NoReuseStrategy.hpp"
#include "udjourney/platform/reuse_strategies/PlatformReuseStrategy.hpp"
#include "udjourney/platform/reuse_strategies/RandomizePositionStrategy.hpp"

struct Resolution {
    int width;
    int height;
    const char *label;
};

// Add these at the top or in your Game class
const int kBaseWidth = 640;
const int kBaseHeight = 480;

const std::vector<Resolution> kResolutions = {{640, 480, "640x480"},
                                              {800, 600, "800x600"},
                                              {1280, 720, "1280x720"},
                                              {1920, 1080, "1920x1080"},
                                              {2560, 1440, "2560x1440"},
                                              {3840, 2160, "4K"}};

int current_resolution_idx = 0;  // Default to first resolution

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

// Helper function to create player animation controller from JSON
AnimSpriteController create_player_animation_controller() {
    std::string config_path =
        std::string(ASSETS_BASE_PATH) + "animations/player_animations.json";
    return udjourney::loaders::AnimationConfigLoader::load_and_create(
        config_path);
}

}  // namespace

const double kUpdateInterval = 0.0001;
bool is_running = true;
// player is now a member of Game class, not a global

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
        int random_number = std::rand();
        lastx = (random_number % 10) * kOffsetPosXMin;
        lastx2 = random_number % kMaxWidth + kOffsetPosXMin;

        auto ra2 = std::rand();
        if (ra2 % 100 < 20) {
            // 5% EightTurnHorizontalBehaviorStrategy, use ORANGE color
            res.emplace_back(std::make_unique<Platform>(
                iGame,
                rect,
                ORANGE,
                false,
                std::make_unique<RandomizePositionStrategy>()));
            float speed =
                static_cast<float>(std::max(1, random_number % 11) / 10.0F);
            float amplitude =
                static_cast<float>(std::max(100, random_number % 220));
            static_cast<Platform *>(res.back().get())
                ->set_behavior(
                    std::make_unique<EightTurnHorizontalBehaviorStrategy>(
                        speed, amplitude));
        } else {
            res.emplace_back(std::make_unique<Platform>(
                iGame,
                rect,
                BLUE,
                false,
                std::make_unique<RandomizePositionStrategy>()));
            if (ra2 % 100 < 25) {
                // 20% HorizontalBehaviorStrategy
                float speed_x =
                    static_cast<float>(std::max(5, random_number % 30));
                float max_offset = static_cast<float>(
                    std::max(kOffsetPosXMin, random_number % kMaxWidth));
                static_cast<Platform *>(res.back().get())
                    ->set_behavior(std::make_unique<HorizontalBehaviorStrategy>(
                        speed_x, max_offset));
                if (std::rand() % 100 < 80) {
                    static_cast<Platform *>(res.back().get())
                        ->add_feature(
                            std::move(std::make_unique<SpikeFeature>()));
                }
            } else if (ra2 % 100 < 45) {
                // 20% OscillatingSizeBehaviorStrategy
                float speed_x =
                    static_cast<float>(std::max(5, random_number % 30));
                const int kShrinkMinOffset = -100;
                const int kShrinkMaxOffset = 150;
                int min_offset =
                    kShrinkMinOffset +
                    (std::rand() %
                     (1 + std::abs(kShrinkMinOffset)));  // -100 to 0
                int max_offset =
                    std::rand() % (kShrinkMaxOffset + 1);  // 0 to 150
                static_cast<Platform *>(res.back().get())
                    ->set_behavior(
                        std::make_unique<OscillatingSizeBehaviorStrategy>(
                            speed_x,
                            static_cast<float>(min_offset),
                            static_cast<float>(max_offset)));
            }
        }
    }

    // Note: Border collision is now handled by WorldBounds system
    // No need to create physical border platforms

    return res;
}

Game::Game(int iWidth, int iHeight) : IGame() {
    m_rect = Rectangle{
        0, 0, static_cast<float>(iWidth), static_cast<float>(iHeight)};
    m_actors.reserve(10);
    // Initialize world bounds to window size by default
    m_world_bounds.set_bounds(
        0.0f, static_cast<float>(iWidth), 0.0f, static_cast<float>(iHeight));

    // Try to load Level 1, fallback to random generation if it fails
    if (!load_scene(
            udjourney::coreutils::get_assets_path("levels/level1.json"))) {
        std::cout
            << "Warning: Could not load level1.json, using random generation"
            << std::endl;
    }
}

void Game::run() {
#ifndef PLATFORM_DREAMCAST
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
#endif
    InitWindow(static_cast<int>(m_rect.width),
               static_cast<int>(m_rect.height),
               "Up-Down Journey");

    // Create platforms from scene or fallback to random generation
    create_platforms_from_scene();

    // Spawn monsters from scene
    create_monsters_from_scene();

    // Spawn player at scene-defined location or default position
    Vector2 player_spawn_pos{320, 240};  // Default position
    if (m_current_scene) {
        auto spawn_data = m_current_scene->get_player_spawn();
        player_spawn_pos = udjourney::scene::Scene::tile_to_world_pos(
            spawn_data.tile_x, spawn_data.tile_y);
    }

    m_player = std::make_unique<Player>(
        *this,
        Rectangle{player_spawn_pos.x, player_spawn_pos.y, 20, 20},
        m_event_dispatcher,
        create_player_animation_controller());
    m_player->add_observer(static_cast<IObserver *>(this));

    m_actors.emplace_back(
        std::make_unique<Bonus>(*this, Rectangle{300, 300, 20, 20}));

    SetTargetFPS(60);
    m_last_update_time = GetTime();
    m_state = GameState::PLAY;

    m_bonus_manager.add_observer(static_cast<IObserver *>(this));

    auto score_hud =
        std::make_unique<ScoreHUD>(Vector2{10, 50}, m_event_dispatcher);
    m_hud_manager.add_background_hud(std::move(score_hud));

    auto dialog_hud =
        std::make_unique<DialogBoxHUD>(Rectangle{300, 400, 200, 80});

    dialog_hud->set_on_finished_callback([this]() {
        m_state = GameState::PLAY;
        m_hud_manager.pop_foreground_hud();
    });

    dialog_hud->set_on_next_callback(
        [this]() { udjourney::Logger::info("Next page in dialog box"); });

    m_hud_manager.push_foreground_hud(std::move(dialog_hud));

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
#ifndef PLATFORM_DREAMCAST
    // Press F1/F2 to cycle through resolutions
    if (IsKeyPressed(KEY_F1)) {
        current_resolution_idx =
            (current_resolution_idx + 1) % kResolutions.size();
        SetWindowSize(kResolutions[current_resolution_idx].width,
                      kResolutions[current_resolution_idx].height);
    }
    if (IsKeyPressed(KEY_F2)) {
        current_resolution_idx =
            (current_resolution_idx - 1 + kResolutions.size()) %
            kResolutions.size();
        SetWindowSize(kResolutions[current_resolution_idx].width,
                      kResolutions[current_resolution_idx].height);
    }
#endif

    // Press 'B' to quit
    bool bPressed = input_mapping.pressed_B();
    if (bPressed) {
        // Press b to quit
        is_running = false;
    }

    if (m_hud_manager.has_focus()) {
        m_state = GameState::PAUSE;  // Pause the game if HUD has focus
        // If the HUD has focus, handle input there
        m_hud_manager.handle_input();
        return;  // Skip further input processing
    }

    // Pause / Unpause the game
    auto start_pressed = input_mapping.pressed_start();
    if (start_pressed) {
        if (m_state == GameState::GAMEOVER) {
            // Set a counter of invicibility for the player when starting a new
            // game
            m_player->set_invicibility(1.8F);
            m_state = GameState::PLAY;
            return;
        }

        if (m_state == GameState::WIN) {
            // Restart the level
            restart_level();
            return;
        }

        if (m_state == GameState::PLAY) {
            m_state = GameState::PAUSE;
        } else if (m_state == GameState::PAUSE && !m_showing_level_select) {
            m_state = GameState::PLAY;
        }
    }

    // Handle level selection input when paused (but only if level select menu
    // is not shown)
    if (m_state == GameState::PAUSE && !m_showing_level_select) {
        bool level_select_pressed = false;
#ifdef PLATFORM_DREAMCAST
        level_select_pressed =
            IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
#else
        level_select_pressed = IsKeyPressed(KEY_L);
#endif
        if (level_select_pressed) {
            show_level_select_menu();
        }
    }

    if (m_state == GameState::PLAY) {
        for (auto &actor : m_actors) {
            actor->process_input();
        }
        m_player->process_input();
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
    DrawText("Press START to resume\n", 300, 40, 20, WHITE);
    DrawText("Press L to load level\n", 300, 70, 20, WHITE);
    DrawText("Press B button to quit\n", 300, 100, 20, RED);
}

void draw_win_screen_() {
    // Draw celebratory win screen
    DrawText("CONGRATULATIONS!", 130, 100, 40, GOLD);
    DrawText("YOU COMPLETED LEVEL 1!", 180, 150, 25, GREEN);
    DrawText(
        "You successfully journeyed from top to bottom!", 120, 200, 20, WHITE);

    DrawText("Press START to play again", 220, 280, 20, YELLOW);
    DrawText("Press B to quit", 280, 320, 20, RED);

    // Draw some celebratory effects
    static float celebration_timer = 0.0f;
    celebration_timer += GetFrameTime();

    // Pulsating stars effect
    for (int i = 0; i < 20; i++) {
        float x = 50 + (i * 30) % 600;
        float y = 50 + ((i * 17) % 300) + sin(celebration_timer * 3 + i) * 10;
        Color star_color = {
            255,
            255,
            0,
            (unsigned char)(128 + 127 * sin(celebration_timer * 2 + i))};
        DrawText("*", static_cast<int>(x), static_cast<int>(y), 20, star_color);
    }
}

void Game::draw_finish_line_() const {
    if (!m_current_scene || m_level_height <= 0) {
        return;  // No level loaded or invalid level height
    }

    // Calculate the finish line Y position (98% of level height where win
    // triggers)
    float win_threshold = m_level_height * 0.98f;

    // Convert world coordinates to screen coordinates relative to game camera
    Rectangle game_rect = get_rectangle();
    float line_y = win_threshold - game_rect.y;

    // Only draw if the line is potentially visible on screen
    if (line_y >= -10 && line_y <= game_rect.height + 10) {
        // Draw a thick pink/magenta finish line across the entire width
        float line_thickness = 4.0f;
        Rectangle finish_line = {
            0,                            // Start from left edge
            line_y - line_thickness / 2,  // Center the line thickness
            game_rect.width,              // Full width of screen
            line_thickness};

        // Draw the main pink line
        DrawRectangleRec(finish_line, MAGENTA);

        // Add some visual flair with a slight glow effect
        DrawRectangleLinesEx(finish_line, 1.0f, PINK);

        // Add text label if line is visible in a reasonable area
        if (line_y >= 50 && line_y <= game_rect.height - 50) {
            const char *finish_text = "FINISH LINE";
            int text_width = MeasureText(finish_text, 16);
            DrawText(finish_text,
                     static_cast<int>(game_rect.width - text_width - 10),
                     static_cast<int>(line_y - 25),
                     16,
                     MAGENTA);
        }
    }
}

bool Game::should_continue_scrolling_() const noexcept {
    // If no scene is loaded or no level height is set, continue scrolling
    if (!m_current_scene || m_level_height <= 0) {
        return true;
    }

    // Calculate finish line Y position (98% of level height where win triggers)
    float win_threshold = m_level_height * 0.98f;

    // Convert world coordinates to screen coordinates relative to game camera
    Rectangle game_rect = get_rectangle();
    float finish_line_screen_y = win_threshold - game_rect.y;

    // Stop scrolling when finish line would be at middle of screen
    // (240px for 480px height)
    float screen_middle = game_rect.height / 2.0f;

    // Continue scrolling if finish line is still above the middle of screen
    return finish_line_screen_y > screen_middle;
}

// Scene system implementations

void Game::draw() const {
    BeginDrawing();
    ClearBackground(SKYBLUE);  // Clear the background with a blue sky color

    // Calculate scale factor
    float scale_x = GetScreenWidth() / static_cast<float>(kBaseWidth);
    float scale_y = GetScreenHeight() / static_cast<float>(kBaseHeight);
    float scale = std::min(scale_x, scale_y);  // Uniform scaling

    // Center the scene if window is not proportional
    float offset_x = (GetScreenWidth() - kBaseWidth * scale) * 0.5F;
    float offset_y = (GetScreenHeight() - kBaseHeight * scale) * 0.5F;

    rlPushMatrix();
    rlTranslatef(offset_x, offset_y, 0);
    rlScalef(scale, scale, 1.0F);

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
            m_player->draw();

            // Draw finish line for level-based gameplay
            if (m_current_scene && m_level_height > 0) {
                draw_finish_line_();
            }
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
        case GameState::WIN:
            draw_win_screen_();
            break;
    }
    m_hud_manager.draw();

    rlPopMatrix();
    DrawText(kResolutions[current_resolution_idx].label, 10, 10, 20, YELLOW);

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
        m_player->update(0.0F);

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
                        // Let the platform handle its own reuse strategy
                        // Scene-based platforms have no reuse strategy
                        // (nullptr) Random platforms have
                        // RandomizePositionStrategy
                        Platform &platform_ref =
                            static_cast<Platform &>(*actor);
                        platform_ref.reuse();

                        // If platform is still CONSUMED after reuse attempt,
                        // it should be removed (no reuse strategy or reuse
                        // failed)
                        if (actor->get_state() == ActorState::CONSUMED) {
                            to_remove.push_back(actor.get());
                        }
                    } break;
                    case static_cast<uint8_t>(ActorType::BONUS): {
                        to_remove.push_back(actor.get());
                    } break;
                    case 3: {  // Monster group ID
                        to_remove.push_back(actor.get());
                    } break;
                    default:
                        // Unknown actor type, remove it to prevent infinite
                        // loops
                        to_remove.push_back(actor.get());
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

    if (m_state == GameState::PLAY) {
        m_bonus_manager.update(delta);
        if (cur_update_time - last_update_time > kUpdateInterval) {
            // Only scroll if finish line hasn't reached middle of screen
            if (should_continue_scrolling_()) {
                m_rect.y += 1;
            }
            for (auto &actor : m_actors) {
                actor->update(delta);
            }
            m_player->update(delta);
            last_update_time = cur_update_time;

            // Check win condition: player reached bottom of level
            if (m_current_scene && m_player) {
                Rectangle player_rect = m_player->get_rectangle();
                float player_bottom = player_rect.y + player_rect.height;

                // Win if player reaches 98% of the level height (very close to
                // actual bottom) This ensures player actually reaches the final
                // platform area before winning
                if (player_bottom >= m_level_height * 0.98f) {
                    m_state = GameState::WIN;
                }
            }
        }
        m_player->handle_collision(m_actors);

        // Handle collision for all monsters
        for (auto &actor : m_actors) {
            if (actor->get_group_id() == 3) {  // Monster group ID
                Monster *monster = dynamic_cast<Monster *>(actor.get());
                if (monster) {
                    monster->handle_collision(m_actors);
                }
            }
        }
    }

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
    const int16_t kModeAttack = 99;

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

    switch (mode) {
        case kModeGameOuver: {
            if (m_state == GameState::GAMEOVER) {
                return;
            }
            m_state = GameState::GAMEOVER;
            auto *comp = m_hud_manager.get_component_by_type("ScoreHUD");
            if (comp != nullptr) {
                auto *score_hud = static_cast<ScoreHUD *>(comp);
                m_score_history.add_score(score_hud->get_score());
                score_hud->set_score(0);  // Reset HUD
            }
        } break;
        case kModeScoring:
            // Parsing scoring event
            std::getline(str_stream, token, ';');
            if (std::optional<int16_t> score_inc_opt = extract_number_(token);
                score_inc_opt.has_value()) {
                m_score += score_inc_opt.value();
                std::cout << "Score updated: +" << score_inc_opt.value()
                          << " (Total: " << m_score << ")" << std::endl;
            }
            break;
        case kModeBonus:
            // Parsing bonus event
            process_bonus_(*this, str_stream);
            break;
        case kModeDash:

            // Parsing dash event
            std::getline(str_stream, token, ';');
            udjourney::Logger::debug(" dash event : %", token);
            if (std::optional<int16_t> dash_opt = extract_number_(token);
                dash_opt.has_value()) {
                dash_fud.dashable = dash_opt.value();
            }
            break;
        case kModeAttack:
            // Player attack - damage nearby monsters
            attack_nearby_monsters();
            break;
        default:
            break;
    }
}

void Game::on_checkpoint_reached(float x, float y) const {
    // Update the last checkpoint position
    // Note: We need to make m_last_checkpoint mutable for this to work
    m_last_checkpoint.x = x;
    m_last_checkpoint.y = y;

    // Optional: Add visual/audio feedback here
    // udjourney::Logger::info("Checkpoint reached at %, %", x, y);
}

Player *Game::get_player() const { return m_player.get(); }

// Scene system implementations
bool Game::load_scene(const std::string &filename) {
    m_current_scene = std::make_unique<udjourney::scene::Scene>();
    if (!m_current_scene->load_from_file(filename)) {
        m_current_scene.reset();
        return false;
    }

    // Update world bounds based on scene content
    // Calculate scene bounds by finding the rightmost and bottommost platforms
    float max_x = static_cast<float>(GetScreenWidth());
    float max_y = static_cast<float>(GetScreenHeight());

    const auto &platforms = m_current_scene->get_platforms();
    for (const auto &platform : platforms) {
        Rectangle world_rect =
            udjourney::scene::Scene::tile_to_world_rect(platform.tile_x,
                                                        platform.tile_y,
                                                        platform.width_tiles,
                                                        platform.height_tiles);

        max_x = std::max(max_x, world_rect.x + world_rect.width);
        max_y = std::max(max_y, world_rect.y + world_rect.height);
    }

    // Update world bounds with calculated max values
    m_world_bounds.set_bounds(0.0f, max_x, 0.0f, max_y);

    return true;
}

void Game::create_platforms_from_scene() {
    if (!m_current_scene) {
        // Fallback to original random generation if no scene loaded
        m_actors = init_platforms(*this);
        return;
    }
    m_actors.clear();
    m_level_height = 0.0f;

    const auto &platforms = m_current_scene->get_platforms();

    for (const auto &platform_data : platforms) {
        // Convert tile coordinates to world coordinates
        Rectangle world_rect = udjourney::scene::Scene::tile_to_world_rect(
            platform_data.tile_x,
            platform_data.tile_y,
            platform_data.width_tiles,
            platform_data.height_tiles);

        // Track the lowest platform for win condition
        float platform_bottom = world_rect.y + world_rect.height;
        if (platform_bottom > m_level_height) {
            m_level_height = platform_bottom;
        }

        // Create platform with appropriate color based on behavior
        Color platform_color = BLUE;  // Default color
        switch (platform_data.behavior_type) {
            case udjourney::scene::PlatformBehaviorType::Horizontal:
                platform_color = GREEN;
                break;
            case udjourney::scene::PlatformBehaviorType::EightTurnHorizontal:
                platform_color = ORANGE;
                break;
            case udjourney::scene::PlatformBehaviorType::OscillatingSize:
                platform_color = PURPLE;
                break;
            case udjourney::scene::PlatformBehaviorType::Static:
            default:
                platform_color = BLUE;
                break;
        }

        // Add spikes color indication
        bool has_spikes = false;
        for (auto feature : platform_data.features) {
            if (feature == udjourney::scene::PlatformFeatureType::Spikes) {
                platform_color = RED;
                has_spikes = true;
                break;
            }
        }

        // Scene-based platforms should not be reused to avoid cluttering the
        // screen Default constructor uses nullptr reuse strategy (no reuse)
        auto platform = std::make_unique<Platform>(*this,
                                                   world_rect,
                                                   platform_color,
                                                   false);  // not Y-repeated

        // Set behavior based on platform data
        switch (platform_data.behavior_type) {
            case udjourney::scene::PlatformBehaviorType::Horizontal: {
                float speed = 50.0f;   // Default speed
                float range = 100.0f;  // Default range

                if (platform_data.behavior_params.count("speed")) {
                    speed = platform_data.behavior_params.at("speed") *
                            10.0f;  // Scale for pixels
                }
                if (platform_data.behavior_params.count("range")) {
                    range = platform_data.behavior_params.at("range") *
                            32.0f;  // Convert tiles to pixels
                }

                platform->set_behavior(
                    std::make_unique<HorizontalBehaviorStrategy>(speed, range));
                break;
            }
            case udjourney::scene::PlatformBehaviorType::EightTurnHorizontal: {
                float speed = 1.0f;        // Default speed
                float amplitude = 100.0f;  // Default amplitude

                if (platform_data.behavior_params.count("speed")) {
                    speed = platform_data.behavior_params.at("speed");
                }
                if (platform_data.behavior_params.count("amplitude")) {
                    amplitude = platform_data.behavior_params.at("amplitude") *
                                32.0f;  // Convert tiles to pixels
                }

                platform->set_behavior(
                    std::make_unique<EightTurnHorizontalBehaviorStrategy>(
                        speed, amplitude));
                break;
            }
            case udjourney::scene::PlatformBehaviorType::OscillatingSize: {
                float speed = 2.0f;        // Default speed
                float min_scale = -50.0f;  // Default min scale
                float max_scale = 50.0f;   // Default max scale

                if (platform_data.behavior_params.count("speed")) {
                    speed = platform_data.behavior_params.at("speed");
                }
                if (platform_data.behavior_params.count("min_scale")) {
                    min_scale =
                        (platform_data.behavior_params.at("min_scale") - 1.0f) *
                        world_rect.width;
                }
                if (platform_data.behavior_params.count("max_scale")) {
                    max_scale =
                        (platform_data.behavior_params.at("max_scale") - 1.0f) *
                        world_rect.width;
                }

                platform->set_behavior(
                    std::make_unique<OscillatingSizeBehaviorStrategy>(
                        speed, min_scale, max_scale));
                break;
            }
            case udjourney::scene::PlatformBehaviorType::Static:
            default:
                // No behavior needed for static platforms
                break;
        }

        // Add features
        udjourney::Logger::info("Processing platform at (%, %) with % features",
                                platform_data.tile_x,
                                platform_data.tile_y,
                                platform_data.features.size());
        for (auto feature : platform_data.features) {
            if (feature == udjourney::scene::PlatformFeatureType::Spikes) {
                platform->add_feature(std::make_unique<SpikeFeature>());
                udjourney::Logger::info(
                    "Added spikes feature to platform at (%, %)",
                    platform_data.tile_x,
                    platform_data.tile_y);
            } else if (feature ==
                       udjourney::scene::PlatformFeatureType::Checkpoint) {
                udjourney::Logger::info(
                    "Creating checkpoint platform at tile_x=%, tile_y=%",
                    platform_data.tile_x,
                    platform_data.tile_y);
                platform->add_feature(std::make_unique<CheckpointFeature>());
                // Set checkpoint platform color to distinguish it
                platform_color = ORANGE;
            }
        }

        m_actors.emplace_back(std::move(platform));
    }

    // Note: Border collision is now handled by WorldBounds system
    // No need to create physical border platforms
}

void Game::create_monsters_from_scene() {
    if (!m_current_scene) {
        return;  // No scene loaded, no monsters to spawn
    }

    const auto &monster_spawns = m_current_scene->get_monster_spawns();

    for (const auto &monster_data : monster_spawns) {
        try {
            std::cout << "DEBUG: Creating monster with preset: "
                      << monster_data.preset_name << std::endl;

            // Convert tile coordinates to world coordinates
            Vector2 world_pos = udjourney::scene::Scene::tile_to_world_pos(
                monster_data.tile_x, monster_data.tile_y);

            Rectangle monster_rect = {
                world_pos.x,
                world_pos.y,
                64.0f,  // Monster width
                64.0f   // Monster height
            };

            // First load the monster preset to get animation configuration
            std::unique_ptr<udjourney::MonsterPreset> preset;
            if (!monster_data.preset_name.empty()) {
                preset = udjourney::MonsterPresetLoader::load_preset(
                    monster_data.preset_name + ".json");
            }

            // Get animation preset file from the monster preset
            std::string animation_file =
                "monster_animations.json";  // Default fallback
            if (preset && !preset->animation_preset_file.empty()) {
                animation_file = preset->animation_preset_file;
            }

            // Load monster animation configuration from the preset
            std::string anim_preset_path =
                std::string(ASSETS_BASE_PATH) + "animations/" + animation_file;
            if (!udjourney::coreutils::file_exists(anim_preset_path)) {
                throw std::runtime_error(
                    "Monster animation config file not found: " +
                    anim_preset_path);
            }

            AnimSpriteController monster_anim_controller =
                udjourney::loaders::AnimationConfigLoader::load_and_create(
                    anim_preset_path);

            std::cout << "DEBUG: Creating monster..." << std::endl;

            // Create monster with EventDispatcher
            auto monster =
                std::make_unique<Monster>(*this,
                                          monster_rect,
                                          std::move(monster_anim_controller),
                                          m_event_dispatcher);

            std::cout << "DEBUG: Monster created successfully!" << std::endl;

            // Load preset if specified (this will apply all preset data)
            if (!monster_data.preset_name.empty()) {
                monster->load_preset(monster_data.preset_name);
            }

            // Configure monster behavior ranges
            float patrol_min = world_pos.x - (monster_data.patrol_range / 2.0f);
            float patrol_max = world_pos.x + (monster_data.patrol_range / 2.0f);
            monster->set_patrol_range(patrol_min, patrol_max);
            monster->set_chase_range(monster_data.chase_range);
            monster->set_attack_range(monster_data.attack_range);

            udjourney::Logger::info(
                "Spawned monster at tile (%, %), world pos (%, %)",
                monster_data.tile_x,
                monster_data.tile_y,
                world_pos.x,
                world_pos.y);

            // Register the monster as an observable with the game (like Player
            // and BonusManager)
            monster->add_observer(static_cast<IObserver *>(this));

            m_actors.emplace_back(std::move(monster));
        } catch (const std::exception &e) {
            std::cerr << "ERROR: Failed to create monster: " << e.what()
                      << std::endl;
            continue;
        } catch (...) {
            std::cerr << "ERROR: Unknown exception while creating monster"
                      << std::endl;
            continue;
        }
    }

    udjourney::Logger::info("Spawned % monsters from scene",
                            monster_spawns.size());
}

void Game::restart_level() {
    // Reset game state
    m_state = GameState::PLAY;

    // Recreate platforms from scene
    create_platforms_from_scene();

    // Respawn monsters from scene
    create_monsters_from_scene();

    // Reset player position

    // Set initial checkpoint if starting fresh
    if (m_current_scene) {
        auto spawn_data = m_current_scene->get_player_spawn();
        m_last_checkpoint = udjourney::scene::Scene::tile_to_world_pos(
            spawn_data.tile_x, spawn_data.tile_y);
    }

    // Reset player at last checkpoint
    if (m_player) {
        m_player->set_rectangle(
            Rectangle{m_last_checkpoint.x, m_last_checkpoint.y, 20, 20});
        m_player->set_invicibility(1.8f);  // Brief invincibility after restart
    }

    // Reset game rect position
    m_rect.y = 0;
}

void Game::show_level_select_menu() {
    m_showing_level_select = true;

    // Create level select HUD
    Rectangle menu_rect = {
        m_rect.width * 0.2f,    // 20% from left
        m_rect.height * 0.15f,  // 15% from top
        m_rect.width * 0.6f,    // 60% width
        m_rect.height * 0.7f    // 70% height
    };

    std::string levels_dir = udjourney::coreutils::get_assets_path("levels");
    auto level_select_hud =
        std::make_unique<LevelSelectHUD>(menu_rect, levels_dir);

    // Set callbacks
    level_select_hud->set_on_level_selected_callback(
        [this](const std::string &level_path) {
            on_level_selected(level_path);
        });

    level_select_hud->set_on_cancelled_callback(
        [this]() { on_level_select_cancelled(); });

    // Add to HUD manager
    m_hud_manager.push_foreground_hud(std::move(level_select_hud));
}

void Game::hide_level_select_menu() {
    m_showing_level_select = false;
    m_hud_manager.pop_foreground_hud();
}

void Game::on_level_selected(const std::string &level_path) {
    // Hide the level select menu
    hide_level_select_menu();

    // Load the selected level
    if (load_scene(level_path)) {
        // Successfully loaded new level - restart with new level
        restart_level();
        std::cout << "Loaded level: " << level_path << std::endl;
    } else {
        std::cerr << "Failed to load level: " << level_path << std::endl;
        // Still return to game even if level failed to load
        m_state = GameState::PLAY;
    }
}

void Game::on_level_select_cancelled() {
    // Hide the level select menu and return to pause menu
    hide_level_select_menu();
}

void Game::attack_nearby_monsters() {
    if (!m_player) return;

    const float ATTACK_RANGE = 150.0f;
    const float ATTACK_DAMAGE = 100.0f;  // Enough to kill most monsters

    Rectangle player_rect = m_player->get_rectangle();
    Vector2 player_center = {player_rect.x + player_rect.width / 2.0f,
                             player_rect.y + player_rect.height / 2.0f};

    std::cout << "Player attack! Looking for monsters within " << ATTACK_RANGE
              << " pixels..." << std::endl;

    int monsters_hit = 0;

    // Check all actors for monsters
    for (auto &actor : m_actors) {
        if (actor->get_group_id() == 3) {  // Monster group ID
            Monster *monster = dynamic_cast<Monster *>(actor.get());
            if (monster) {
                Rectangle monster_rect = monster->get_rectangle();
                Vector2 monster_center = {
                    monster_rect.x + monster_rect.width / 2.0f,
                    monster_rect.y + monster_rect.height / 2.0f};

                // Calculate distance between player and monster
                float dx = player_center.x - monster_center.x;
                float dy = player_center.y - monster_center.y;
                float distance = sqrt(dx * dx + dy * dy);

                if (distance <= ATTACK_RANGE) {
                    std::cout << "Attacking monster at distance " << distance
                              << std::endl;
                    monster->take_damage(ATTACK_DAMAGE);
                    monsters_hit++;
                }
            }
        }
    }

    if (monsters_hit == 0) {
        std::cout << "No monsters in range to attack." << std::endl;
    } else {
        std::cout << "Hit " << monsters_hit << " monsters!" << std::endl;
    }
}
