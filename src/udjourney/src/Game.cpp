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
#include <cstdio>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "udjourney/widgets/ScrollableListWidget.hpp"

#include <udj-core/CoreUtils.hpp>
#include <udj-core/Logger.hpp>

#include "udjourney/Bonus.hpp"
#include "udjourney/Monster.hpp"
#include "udjourney/Player.hpp"
#include "udjourney/Projectile.hpp"
#include "udjourney/AnimSpriteController.hpp"
#include "udjourney/SpriteAnim.hpp"
#include "udjourney/ScoreHistory.hpp"
#include "udjourney/core/events/ScoreEvent.hpp"
#include "udjourney/core/events/WeaponSelectedEvent.hpp"
#include "udjourney/core/events/HealthChangedEvent.hpp"
#include "udjourney/hud/DialogBoxHUD.hpp"
#include "udjourney/hud/HUDComponent.hpp"
#include "udjourney/ActionDispatcher.hpp"
#include "udjourney/widgets/IWidget.hpp"
#include "udjourney/widgets/WidgetFactory.hpp"
#include "udjourney/factories/ActorFactory.hpp"
#include "udjourney/factories/PlatformFactory.hpp"
#include "udjourney/factories/UiFactory.hpp"
#include "udjourney/hud/GameMenuHUD.hpp"
#include "udjourney/hud/LevelSelectHUD.hpp"
#include "udjourney/render/StateRenderers.hpp"
#include "udjourney/hud/scene/ScoreDisplayHUD.hpp"
#include "udjourney/hud/scene/HeartHealthHUD.hpp"
#include "udjourney/hud/scene/WeaponHUD.hpp"
#include "udjourney/interfaces/IActor.hpp"
#include "udjourney/managers/TextureManager.hpp"
#include "udjourney/loaders/AnimationConfigLoader.hpp"
#include "udjourney/platform/Platform.hpp"
#include "udjourney/platform/behavior_strategies/EightTurnHorizontalBehaviorStrategy.hpp"
#include "udjourney/platform/behavior_strategies/HorizontalBehaviorStrategy.hpp"
#include "udjourney/platform/behavior_strategies/OscillatingSizeBehaviorStrategy.hpp"
#include "udjourney/platform/features/CheckpointFeature.hpp"
#include "udjourney/platform/features/PlatformFeatureBase.hpp"
#include "udjourney/components/HealthComponent.hpp"
#include "udjourney/platform/features/SpikeFeature.hpp"
#include "udjourney/platform/reuse_strategies/NoReuseStrategy.hpp"
#include "udjourney/platform/reuse_strategies/PlatformReuseStrategy.hpp"
#include "udjourney/platform/reuse_strategies/RandomizePositionStrategy.hpp"
#include "udjourney/commands/CallbackCommand.hpp"

using udj::core::filesystem::file_exists;
using udj::core::filesystem::get_assets_path;

namespace udjourney {

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

// Local helper functions at file scope (no namespace)
DashHud dash_hud;

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

    register_core_event_handlers_();

    // Register menu action callbacks
    register_menu_actions();

    // Initialize state renderers
    init_state_renderers_();

    // Load particle presets
    if (!m_particle_preset_loader.load_from_file("particles.json")) {
        udj::core::Logger::warning("Warning: Could not load particles.json");
    }

    // Load title screen scene
    if (!load_scene(udjourney::coreutils::get_assets_path(
            "levels/title_screen.json"))) {
        udj::core::Logger::error("ERROR: Could not load title_screen.json");
    }
}

void Game::run() {
#ifndef PLATFORM_DREAMCAST
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
#endif
    InitWindow(static_cast<int>(m_rect.width),
               static_cast<int>(m_rect.height),
               "Up-Down Journey");

    // Load menu configuration
    m_menu_manager.load_config("menu_config.json");

    create_huds_from_scene();
    SetTargetFPS(60);
    m_last_update_time = GetTime();

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

    // Pause / Unpause the game - show game menu on pause
    auto start_pressed = input_mapping.pressed_start();
    if (start_pressed) {
        if (m_state == GameState::PLAY) {
            show_game_menu();  // Show menu instead of just pausing
        } else if (m_state == GameState::PAUSE &&
                   !m_level_select_manager.is_showing() &&
                   !m_menu_manager.is_showing()) {
            m_state = GameState::PLAY;
        }
    }

    if (m_state == GameState::PLAY) {
        for (auto &actor : m_actors) {
            actor->process_input();
        }
        m_player->process_input();

        // Handle shooting input (X button / E key)
        bool shoot_pressed = false;
#ifdef PLATFORM_DREAMCAST
        shoot_pressed =
            IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
#else
        shoot_pressed =
            IsKeyPressed(KEY_E) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
#endif
        if (shoot_pressed) {
            udj::core::Logger::debug(
                "E key pressed! Player: % Can shoot: %",
                (m_player ? "yes" : "no"),
                (m_player && m_player->can_shoot() ? "yes" : "no"));
        }
        if (shoot_pressed && m_player && m_player->can_shoot()) {
            m_player->execute_command("shoot_projectile");
        }

        // Handle projectile type cycling (C key / Y button)
        bool cycle_pressed = false;
#ifdef PLATFORM_DREAMCAST
        cycle_pressed = IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP);
#else
        cycle_pressed = IsKeyPressed(KEY_C);
#endif
        if (cycle_pressed && m_player) {
            m_player->cycle_projectile_type();
        }
    }
}
void Game::clear_scene() {
    // Remove all widgets from m_actors
    m_actors.erase(std::remove_if(m_actors.begin(),
                                  m_actors.end(),
                                  [](const std::unique_ptr<IActor> &actor) {
                                      return actor->get_group_id() ==
                                             4;  // Remove widgets
                                  }),
                   m_actors.end());

    // Clear background HUDs
    m_hud_manager.clear_background_huds();

    m_actors.clear();
    m_pending_actors.clear();
    m_updating_actors = false;
    m_selected_widget_index = 0;
    m_frames_since_scene_load = 0;
    m_background_manager.reset_ui_scroll();
    m_scene_huds.clear();

    // Only reset player if not in GAMEOVER/WIN state
    // (in those states, player destruction is deferred until after collision
    // processing)
    if (m_state != GameState::GAMEOVER && m_state != GameState::WIN) {
        udj::core::Logger::debug(
            "clear_scene: Resetting player (state is not GAMEOVER/WIN)");
        m_player.reset();
    } else {
        udj::core::Logger::debug(
            "clear_scene: NOT resetting player (state is GAMEOVER or WIN - "
            "will be deferred)");
    }
}

void Game::register_core_event_handlers_() {
    // Core game-wide handlers that should persist across scenes.
    // Scene-specific handlers (HUD listeners, etc.) are expected to be
    // registered by their respective objects and are cleared on scene switch.
    m_event_dispatcher.register_handler(
        udjourney::core::events::ScoreEvent::TYPE,
        [this](const udjourney::core::events::IEvent &evt) {
            const auto &score_ev =
                static_cast<const udjourney::core::events::ScoreEvent &>(evt);
            m_score += score_ev.value;
        });
}

/**
 * Applies the currently loaded scene by creating platforms, monsters,
 * and HUDs defined in the scene.
 * If no scene is loaded, logs an error message.
 */
void Game::apply_current_scene(SceneApplyMode mode) {
    m_event_dispatcher.clear_all_handlers();
    register_core_event_handlers_();
    if (!m_current_scene) {
        udj::core::Logger::error("No current scene to apply!");
        return;
    }

    SceneApplyMode resolved = mode;
    if (resolved == SceneApplyMode::Auto) {
        resolved = (m_current_scene->get_type() ==
                    udjourney::scene::SceneType::UiScreen)
                       ? SceneApplyMode::UiScreen
                       : SceneApplyMode::Gameplay;
    }

    clear_scene();

    // Gameplay scenes: world + UI
    if (resolved == SceneApplyMode::Gameplay) {
        initialize_gameplay();
        return;
    }

    m_rect.y = 0;                 // Reset camera
    m_selected_widget_index = 0;  // Reset widget selection
    m_frames_since_scene_load = 0;
    m_background_manager.reset_ui_scroll();
    m_scene_huds.clear();
    m_actors.clear();
    m_pending_actors.clear();
    m_updating_actors = false;
    m_selected_widget_index = 0;

    create_huds_from_scene();
}

/**
 * Loads a scene from the specified filename and applies it immediately.
 * Returns true if the scene was loaded and applied successfully, false
 * otherwise.
 * @param filename The path to the scene file to load.
 * @return True if the scene was loaded and applied successfully, false
 * otherwise.
 */
bool Game::load_and_apply_scene(const std::string &filename) {
    if (!load_scene(filename)) {
        return false;
    }
    apply_current_scene(SceneApplyMode::Auto);
    return true;
}

// Removed: draw_pause_() moved to PauseStateRenderer

void Game::draw_finish_line_() const {
    if (!m_current_scene || m_level_height <= 0) {
        return;  // No level loaded or invalid level height
    }

    // Calculate the finish line Y position (98% of level height where win
    // triggers)
    float win_threshold = m_level_height * 1.00f;

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

void Game::draw_backgrounds_() const {
    if (!m_current_scene) {
        return;
    }

    const bool use_ui_scroll =
        (m_state == GameState::TITLE || m_state == GameState::GAMEOVER ||
         m_state == GameState::WIN);

    m_background_manager.draw(m_rect.y,
                              use_ui_scroll,
                              static_cast<float>(kBaseWidth),
                              static_cast<float>(kBaseHeight));
}

void Game::draw_huds_() const {
    for (const auto &hud : m_scene_huds) {
        if (hud) {
            hud->draw();
        }
    }
}

void Game::draw() const {
    BeginDrawing();
    ClearBackground(SKYBLUE);  // Clear the background with a blue sky color

    // Calculate viewport transform
    float scale_x = GetScreenWidth() / static_cast<float>(kBaseWidth);
    float scale_y = GetScreenHeight() / static_cast<float>(kBaseHeight);
    float scale = std::min(scale_x, scale_y);  // Uniform scaling

    float offset_x = (GetScreenWidth() - kBaseWidth * scale) * 0.5F;
    float offset_y = (GetScreenHeight() - kBaseHeight * scale) * 0.5F;

    rlPushMatrix();
    rlTranslatef(offset_x, offset_y, 0);
    rlScalef(scale, scale, 1.0F);

    // Delegate rendering to current state renderer
    auto it = m_state_renderers.find(m_state);
    if (it != m_state_renderers.end()) {
        it->second->render(*this);
    }

    // Always draw HUD manager on top
    m_hud_manager.draw();

    // Draw FUDs (Fixed UI Displays) from current scene
    // Skip for states that handle their own FUD/widget drawing
    if (m_current_scene && m_state != GameState::TITLE &&
        m_state != GameState::GAMEOVER && m_state != GameState::WIN) {
        draw_huds_();
    }

    rlPopMatrix();

    // Draw left and right borders
    // Left border (from screen left to game area left)
    if (offset_x > 0) {
        DrawRectangle(
            0, 0, static_cast<int>(offset_x), GetScreenHeight(), BLACK);
    }
    // Right border (from game area right to screen right)
    if (offset_x > 0) {
        DrawRectangle(static_cast<int>(offset_x + kBaseWidth * scale),
                      0,
                      static_cast<int>(offset_x + 1),
                      GetScreenHeight(),
                      BLACK);
    }

    DrawText(kResolutions[current_resolution_idx].label, 10, 10, 20, YELLOW);

    EndDrawing();
}

void Game::update() {
    static double last_update_time = 0.0;

    // Update background scroll for UI screens
    if (m_current_scene &&
        m_current_scene->get_type() == udjourney::scene::SceneType::UiScreen) {
        m_background_manager.update_ui_scroll(GetFrameTime(),
                                              static_cast<float>(kBaseHeight));
    }

    // Update particle system
    m_particle_manager.update(GetFrameTime());

    // Widget input handling for TITLE, WIN, and GAMEOVER states
    if (m_state == GameState::TITLE || m_state == GameState::WIN ||
        m_state == GameState::GAMEOVER) {
        // Increment frame counter
        m_frames_since_scene_load++;

        // Skip input for first 3 frames after scene load to prevent key
        // bleed-through
        if (m_frames_since_scene_load < 3) {
            // Still update widget focus visuals but don't process activation
            std::vector<IWidget *> widgets;
            for (const auto &actor : m_actors) {
                if (!actor) continue;
                if (actor->get_group_id() == 4) {
                    IWidget *widget = static_cast<IWidget *>(actor.get());
                    if (widget && widget->is_selectable()) {
                        widgets.push_back(widget);
                    }
                }
            }
            for (size_t i = 0; i < widgets.size(); ++i) {
                widgets[i]->set_focused(static_cast<int>(i) ==
                                        m_selected_widget_index);
            }
        } else {
            // Handle keyboard input first (doesn't require collecting widgets)
            bool keyboard_input_handled = false;

            if (IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_S) ||
                IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN) ||
                IsKeyPressed(KEY_PAGE_UP) || IsKeyPressed(KEY_PAGE_DOWN) ||
                GetMouseWheelMove() != 0.0f) {
                // Collect widgets for keyboard/wheel input
                std::vector<IWidget *> widgets;
                for (const auto &actor : m_actors) {
                    if (!actor) continue;
                    if (actor->get_group_id() == 4) {
                        IWidget *widget = static_cast<IWidget *>(actor.get());
                        if (widget && widget->is_selectable()) {
                            widgets.push_back(widget);
                        }
                    }
                }

                if (!widgets.empty()) {
                    // Keyboard navigation: Z = up, S = down
                    if (IsKeyPressed(KEY_Z)) {
                        m_selected_widget_index =
                            (m_selected_widget_index - 1 +
                             static_cast<int>(widgets.size())) %
                            static_cast<int>(widgets.size());
                    }
                    if (IsKeyPressed(KEY_S)) {
                        m_selected_widget_index =
                            (m_selected_widget_index + 1) %
                            static_cast<int>(widgets.size());
                    }

                    // Update focus state for all widgets
                    for (size_t i = 0; i < widgets.size(); ++i) {
                        widgets[i]->set_focused(static_cast<int>(i) ==
                                                m_selected_widget_index);
                    }

                    // Handle ScrollableListWidget-specific input FIRST (before
                    // Enter activation) This allows Up/Down to change selection
                    // before Enter loads a level Only process list input if the
                    // list widget is focused
                    bool list_input_handled = false;
                    for (auto &actor : m_actors) {
                        if (!actor) continue;
                        if (actor->get_group_id() == 4) {
                            if (auto *list =
                                    dynamic_cast<ScrollableListWidget *>(
                                        actor.get())) {
                                // Only handle list input if this list widget is
                                // focused
                                if (list->is_focused()) {
                                    if (IsKeyPressed(KEY_UP)) {
                                        list->scroll_up();
                                        list_input_handled = true;
                                    }
                                    if (IsKeyPressed(KEY_DOWN)) {
                                        list->scroll_down();
                                        list_input_handled = true;
                                    }
                                    if (IsKeyPressed(KEY_PAGE_UP)) {
                                        list->page_up();
                                        list_input_handled = true;
                                    }
                                    if (IsKeyPressed(KEY_PAGE_DOWN)) {
                                        list->page_down();
                                        list_input_handled = true;
                                    }

                                    float wheel = GetMouseWheelMove();
                                    if (wheel > 0) {
                                        list->scroll_up();
                                        list_input_handled = true;
                                    } else if (wheel < 0) {
                                        list->scroll_down();
                                        list_input_handled = true;
                                    }
                                }
                            }
                        }
                    }

                    // Only activate widget with Enter/Space if no list
                    // navigation happened This prevents immediate activation
                    // when transitioning screens
                    if (!list_input_handled &&
                        (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))) {
                        IWidget *focused_widget =
                            widgets[m_selected_widget_index];

                        // Only activate if the focused widget is NOT a
                        // ScrollableListWidget or if it IS a
                        // ScrollableListWidget (to load selected level)
                        focused_widget->on_click();
                        keyboard_input_handled = true;

                        // Return immediately - action may have changed state
                        return;
                    }
                }
            }

            // Mouse input is disabled - keyboard-only navigation
        }  // End frame skip check
    }

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

        // Process all queued notifications AFTER all updates complete
        // but BEFORE removing dead actors
        process_pending_notifications();
    } else if (m_state == GameState::TITLE || m_state == GameState::WIN ||
               m_state == GameState::GAMEOVER) {
        // Update widgets for animations (e.g., ScrollableListWidget scroll
        // animation)
        float delta = GetFrameTime();
        for (auto &actor : m_actors) {
            if (actor && actor->get_group_id() == 4) {  // Widget group ID
                actor->update(delta);
            }
        }
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

    // Only update game time when not paused
    double cur_update_time = GetTime();
    auto delta = static_cast<float>(cur_update_time - last_update_time);

    if (m_state != GameState::PAUSE) {
        if (m_state == GameState::PLAY) {
            m_bonus_manager.update(delta);
            if (cur_update_time - last_update_time > kUpdateInterval) {
                // Only scroll if finish line hasn't reached middle of screen
                if (should_continue_scrolling_()) {
                    // Use scroll speed from current scene, default to 1.0 if no
                    // scene
                    float scroll_speed =
                        m_current_scene ? m_current_scene->get_scroll_speed()
                                        : 1.0f;
                    m_rect.y += scroll_speed;
                }
                for (auto &actor : m_actors) {
                    actor->update(delta);
                }
                if (m_player) m_player->update(delta);
                last_update_time = cur_update_time;

                // Check projectile-monster collisions
                for (auto &proj_actor : m_actors) {
                    // Not a projectile
                    if (!proj_actor || proj_actor->get_group_id() != 5)
                        continue;

                    auto *projectile =
                        dynamic_cast<udjourney::Projectile *>(proj_actor.get());
                    if (!projectile || !projectile->is_alive()) continue;

                    Rectangle proj_rect = projectile->get_rectangle();

                    for (auto &monster_actor : m_actors) {
                        // Not a monster
                        if (!monster_actor ||
                            monster_actor->get_group_id() != 3)
                            continue;

                        auto *monster =
                            dynamic_cast<Monster *>(monster_actor.get());
                        if (!monster || !monster->is_alive()) continue;

                        // Skip if monster is already dying/dead
                        if (monster->get_state() == ActorState::CONSUMED) {
                            continue;
                        }

                        Rectangle monster_rect = monster->get_rectangle();

                        if (CheckCollisionRecs(proj_rect, monster_rect)) {
                            // Hit! Monster takes damage from projectile
                            udj::core::Logger::info(
                                "Projectile hit monster! Damage: " +
                                std::to_string(projectile->get_damage()) +
                                " Monster ptr: " +
                                std::to_string(
                                    reinterpret_cast<uintptr_t>(monster)));

                            if (monster) {
                                udj::core::Logger::info(
                                    "Calling monster->take_damage...");
                                monster->take_damage(static_cast<float>(
                                    projectile->get_damage()));
                                udj::core::Logger::info("take_damage returned");
                            } else {
                                udj::core::Logger::error(
                                    "ERROR: Monster pointer is null!");
                            }

                            // Create sparkle particle effect at hit location
                            const ParticlePreset *sparkle_preset =
                                m_particle_preset_loader.get_preset("sparkle");
                            if (sparkle_preset) {
                                Vector2 hit_pos = {
                                    proj_rect.x + proj_rect.width / 2.0f,
                                    proj_rect.y + proj_rect.height / 2.0f};
                                udj::core::Logger::info(
                                    "Creating impact burst at position: " +
                                    std::to_string(hit_pos.x) + ", " +
                                    std::to_string(hit_pos.y));
                                m_particle_manager.create_burst(*sparkle_preset,
                                                                hit_pos);
                                udj::core::Logger::info(
                                    "Particle count: " +
                                    std::to_string(
                                        m_particle_manager
                                            .get_total_particle_count()));
                            } else {
                                udj::core::Logger::error(
                                    "ERROR: Could not find 'impact' preset!");
                            }

                            projectile->destroy();
                            udj::core::Logger::info(
                                "Projectile destroyed after hit");
                            break;
                        }
                    }
                }

                // Remove dead projectiles and consumed monsters
                m_actors.erase(
                    std::remove_if(
                        m_actors.begin(),
                        m_actors.end(),
                        [](const std::unique_ptr<IActor> &actor) {
                            if (actor->get_group_id() == 5) {  // Projectile
                                auto *proj =
                                    dynamic_cast<udjourney::Projectile *>(
                                        actor.get());
                                return proj && !proj->is_alive();
                            }
                            if (actor->get_group_id() == 3) {  // Monster
                                // Only remove after death animation completes
                                return actor->get_state() ==
                                       ActorState::CONSUMED;
                            }
                            return false;
                        }),
                    m_actors.end());

                // Check win condition: player reached bottom of level
                if (m_current_scene && m_player) {
                    Rectangle player_rect = m_player->get_rectangle();
                    float player_bottom = player_rect.y + player_rect.height;

                    // Win if player reaches 98% of the level height (very close
                    // to actual bottom) This ensures player actually reaches
                    // the final platform area before winning
                    if (player_bottom >= m_level_height * 1.00f) {
                        m_state = GameState::WIN;

                        // Save final score for display on win screen
                        m_final_score = m_score;

                        // Load win screen with widgets
                        std::string win_path =
                            udjourney::coreutils::get_assets_path(
                                "levels/win_screen.json");
                        if (load_scene(win_path)) {
                            // DON'T destroy player immediately - defer until
                            // after current frame
                            m_hud_manager.clear_background_huds();

                            // Remove all actors except widgets
                            m_actors.erase(
                                std::remove_if(
                                    m_actors.begin(),
                                    m_actors.end(),
                                    [](const std::unique_ptr<IActor> &actor) {
                                        return actor->get_group_id() !=
                                               4;  // Keep widgets only
                                    }),
                                m_actors.end());

                            // Load win screen widgets
                            create_huds_from_scene();
                            m_rect.y = 0;  // Reset camera
                        }
                    }
                }
            }

            // Only handle collisions if player exists and game is in PLAY state
            if (m_player && m_state == GameState::PLAY) {
                udj::core::Logger::debug(
                    "Calling player handle_collision (state=PLAY)");
                m_player->handle_collision(m_actors);
            } else if (m_player) {
                udj::core::Logger::debug(
                    "Skipping player handle_collision (state != PLAY)");
            }

            // Handle collision for all monsters (only during gameplay)
            if (m_state == GameState::PLAY) {
                for (auto &actor : m_actors) {
                    if (actor->get_group_id() == 3) {  // Monster group ID
                        Monster *monster = dynamic_cast<Monster *>(actor.get());
                        if (monster) {
                            monster->handle_collision(m_actors);
                        }
                    }
                }
            }

            // Clean up player after collisions are processed (deferred
            // destruction)
            if ((m_state == GameState::GAMEOVER || m_state == GameState::WIN) &&
                m_player) {
                udj::core::Logger::debug(
                    "Cleaning up player after game over/win");
                m_player.reset();
            }
        }

        m_hud_manager.update(delta);
    } else {
        // Game is paused, reset the timer to avoid time jump when resuming
        last_update_time = cur_update_time;
    }

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
    // Queue the notification for processing at safe time
    m_pending_notifications.push_back(iEvent);
}

void Game::process_pending_notifications() {
    // Process all queued notifications
    for (const auto &event : m_pending_notifications) {
        process_notification_immediate(event);
    }
    m_pending_notifications.clear();
}

void Game::process_notification_immediate(const std::string &iEvent) {
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
            // Save current score to history and final score for display
            m_score_history.add_score(m_score);
            m_final_score = m_score;

            // Load game over screen with widgets
            std::string gameover_path = udjourney::coreutils::get_assets_path(
                "levels/game_over_screen.json");
            if (load_scene(gameover_path)) {
                // DON'T destroy player immediately - defer until after current
                // frame This prevents destroying the player while it's still
                // executing The player will be cleaned up at the end of update
                // loop
                m_hud_manager.clear_background_huds();

                // Remove all actors except widgets
                m_actors.erase(
                    std::remove_if(m_actors.begin(),
                                   m_actors.end(),
                                   [](const std::unique_ptr<IActor> &actor) {
                                       return actor->get_group_id() !=
                                              4;  // Keep widgets only
                                   }),
                    m_actors.end());

                // Load game over screen widgets
                create_huds_from_scene();
                m_rect.y = 0;  // Reset camera
                m_score = 0;   // Reset score for next game
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
                dash_hud.dashable = dash_opt.value();
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

const DashHud &Game::get_dash_hud() const { return dash_hud; }

void Game::init_state_renderers_() {
    m_state_renderers[GameState::TITLE] = std::make_unique<UiScreenRenderer>();
    m_state_renderers[GameState::PLAY] = std::make_unique<PlayStateRenderer>();
    m_state_renderers[GameState::PAUSE] =
        std::make_unique<PauseStateRenderer>();
    m_state_renderers[GameState::GAMEOVER] =
        std::make_unique<UiScreenRenderer>();
    m_state_renderers[GameState::WIN] = std::make_unique<UiScreenRenderer>();
}

// Scene system implementations
bool Game::load_scene(const std::string &filename) {
    // Clear background manager before destroying old scene to avoid dangling
    // pointers
    m_background_manager.clear();

    m_current_scene = std::make_unique<udjourney::scene::Scene>();
    if (!m_current_scene->load_from_file(filename)) {
        m_current_scene.reset();
        m_background_manager.clear();
        return false;
    }

    // Track the current scene filename for restart functionality
    m_current_scene_filename = filename;

    // Track gameplay levels separately so we can restart from game over screen
    if (m_current_scene->get_type() == udjourney::scene::SceneType::Level) {
        m_last_gameplay_level_filename = filename;
    }

    // Bind background system to the new scene (sort layers + preload textures)
    m_background_manager.set_scene(*m_current_scene);

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

void Game::create_huds_from_scene() {
    // Important: Clear scene HUDs FIRST to destroy objects while scene is still
    // valid
    m_scene_huds.clear();

    // Then clear event handlers to prevent dangling callbacks
    m_event_dispatcher.clear_handlers(
        udjourney::core::events::WeaponSelectedEvent::TYPE);

    if (!m_current_scene) {
        return;
    }

    // Remove previously-created scene widgets to avoid duplicates when
    // reloading UI scenes
    m_actors.erase(std::remove_if(m_actors.begin(),
                                  m_actors.end(),
                                  [](const std::unique_ptr<IActor> &actor) {
                                      return actor && actor->get_group_id() ==
                                                          4;  // Widget group
                                  }),
                   m_actors.end());

    auto built = UiFactory::create(*m_current_scene, *this, m_event_dispatcher);

    m_scene_huds = std::move(built.scene_huds);
    for (auto &widget_actor : built.widget_actors) {
        m_actors.push_back(std::move(widget_actor));
    }

    m_selected_widget_index = 0;
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

        // Update level height based on platform bottom
        // It will determine ther end of the level for win condition
        // If this line is removed, the level height will remain 0,
        // winning without playing through the level
        float platform_bottom = world_rect.y + world_rect.height;
        if (platform_bottom > m_level_height) {
            m_level_height = platform_bottom;
        }

        auto platform =
            PlatformFactory::create(*this, world_rect, platform_data);
        m_actors.emplace_back(std::move(platform));
    }
}

void Game::create_monsters_from_scene() {
    if (!m_current_scene) {
        return;  // No scene loaded, no monsters to spawn
    }

    const auto &monster_spawn_data = m_current_scene->get_monster_spawns();
    MonsterFactory factory(*this, m_event_dispatcher);

    // Set physics config from scene
    factory.set_physics_config(m_current_scene->get_physics_config());

    for (const auto &monster_data : monster_spawn_data) {
        try {
            // Use MonsterFactory to create the monster
            auto monster = factory.create_actor_from_monster_data(monster_data);

            // Register the monster as an observable with the game
            if (auto *monster_ptr = dynamic_cast<Monster *>(monster.get())) {
                monster_ptr->add_observer(static_cast<IObserver *>(this));
            }

            m_actors.emplace_back(std::move(monster));
        } catch (const std::exception &e) {
            Logger::error("Failed to create monster: " + std::string(e.what()));
            continue;
        } catch (...) {
            Logger::error("Unknown exception while creating monster");
            continue;
        }
    }

    udjourney::Logger::info("Spawned % monsters from scene",
                            monster_spawn_data.size());
}

void Game::restart_level() {
    // Reload the current scene from file to ensure we have fresh data
    if (!m_current_scene_filename.empty() && m_current_scene) {
        if (!m_current_scene->load_from_file(m_current_scene_filename)) {
            Logger::error("Failed to reload scene for restart: %",
                          m_current_scene_filename);
            return;
        }
    }

    // Reset game state
    m_state = GameState::PLAY;

    // Recreate platforms from scene
    create_platforms_from_scene();

    // Respawn monsters from scene
    create_monsters_from_scene();

    // Recreate player with updated physics config from reloaded scene
    if (m_current_scene) {
        create_player();
        m_player->set_invicibility(1.8f);  // Brief invincibility after restart

        // Create HUD objects from scene
        create_huds_from_scene();
    }

    // Reset game rect position
    m_rect.y = 0;
}

void Game::show_level_select_menu() {
    m_level_select_manager.show(
        [this](const std::string &level_path) {
            on_level_selected(level_path);
        },
        [this]() { on_level_select_cancelled(); });
}

void Game::on_level_selected(const std::string &level_path) {
    // Hide the level select menu first
    m_level_select_manager.hide();

    // Load the selected level
    if (load_scene(level_path)) {
        // Successfully loaded new level - restart with new level
        hide_game_menu();
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
    m_level_select_manager.hide();
}

void Game::show_game_menu() {
    if (m_menu_manager.is_showing()) return;

    m_state = GameState::PAUSE;

    m_menu_manager.show_game_menu(m_current_scene.get(),
                                  [this]() { hide_game_menu(); });
}

void Game::hide_game_menu() {
    if (!m_menu_manager.is_showing()) return;

    m_menu_manager.hide_game_menu();

    // Only resume gameplay if we're still paused.
    // Menu actions may have transitioned the game to a different UI state
    // (e.g., TITLE/LEVEL_SELECT), and we must not overwrite that here.
    if (m_state == GameState::PAUSE) {
        m_state = GameState::PLAY;
    }
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

void Game::register_menu_actions() {
    // Start Game action (restart last gameplay level or load level1 if no level
    // was played)
    ActionDispatcher::register_action(
        "start_game", [](IGame *game, const std::vector<std::string> &params) {
            std::cout << "[ACTION] Start Game triggered" << std::endl;
            auto &g = static_cast<Game &>(*game);

            // If we have a last gameplay level, load and restart it
            if (!g.m_last_gameplay_level_filename.empty()) {
                g.m_state = GameState::PLAY;
                g.load_and_apply_scene(g.m_last_gameplay_level_filename);
            } else {
                // Load level1 scene (for initial game start)
                std::string level_path =
                    udjourney::coreutils::get_assets_path("levels/level1.json");
                g.m_state = GameState::PLAY;
                g.load_and_apply_scene(level_path);
            }
        });

    // Load Level action (format: "load_level:level_name")
    ActionDispatcher::register_action(
        "load_level", [](IGame *game, const std::vector<std::string> &params) {
            if (params.empty()) return;

            // params[0] is the filename (e.g., "level1.json")
            std::string filename = params[0];
            Logger::info("[ACTION] Load Level: " + filename);

            auto &g = static_cast<Game &>(*game);

            std::string level_path =
                udjourney::coreutils::get_assets_path("levels/" + filename);
            g.m_state = GameState::PLAY;
            g.load_and_apply_scene(level_path);
        });

    // Show Level Select action
    ActionDispatcher::register_action(
        "show_level_select",
        [](IGame *game, const std::vector<std::string> &params) {
            Logger::info("[ACTION] ===== Show Level Select triggered =====");
            auto &captured_game = static_cast<Game &>(*game);
            std::string level_select_path =
                udjourney::coreutils::get_assets_path(
                    "levels/level_select_screen.json");
            // Now load the new scene
            if (captured_game.load_scene(level_select_path)) {
                captured_game.m_state = GameState::TITLE;

                // Load level select screen widgets
                // captured_game->load_widgets_from_scene();

                // Set focus to the ScrollableListWidget (should be the
                // first selectable widget) Find the index of the
                // ScrollableListWidget in the selectable widgets
                std::vector<IWidget *> selectable_widgets;
                int list_index = -1;
                for (const auto &actor : captured_game.m_actors) {
                    if (actor && actor->get_group_id() == 4) {
                        IWidget *widget = static_cast<IWidget *>(actor.get());
                        if (widget && widget->is_selectable()) {
                            if (dynamic_cast<ScrollableListWidget *>(widget)) {
                                list_index =
                                    static_cast<int>(selectable_widgets.size());
                            }
                            selectable_widgets.push_back(widget);
                        }
                    }
                }

                // Focus the list widget (or default to 0 if not found)
                captured_game.m_selected_widget_index =
                    (list_index >= 0) ? list_index : 0;

                // Reset frame counter to prevent immediate input
                captured_game.m_frames_since_scene_load = 0;
                captured_game.apply_current_scene();
                Logger::info(
                    "[ACTION] Level select screen loaded with " +
                    std::to_string(captured_game.m_actors.size()) +
                    " actors, focus on widget " +
                    std::to_string(captured_game.m_selected_widget_index));
            }
        });

    // Show Level Select action
    ActionDispatcher::register_action(
        "show_level_select2",
        [](IGame *game, const std::vector<std::string> &params) {
            auto &captured_game = static_cast<Game &>(*game);
            // Don't hide the game menu, just show level select on top
            captured_game.show_level_select_menu();
        });

    // Quit Game action
    ActionDispatcher::register_action(
        "quit_game", [](IGame *game, const std::vector<std::string> &params) {
            Logger::info("[ACTION] Quit Game triggered");
#ifndef PLATFORM_DREAMCAST
            CloseWindow();
#endif
        });

    // Show Options action (placeholder)
    ActionDispatcher::register_action(
        "show_options",
        [](IGame *game, const std::vector<std::string> &params) {
            Logger::info("[ACTION] Show Options triggered (not implemented)");
        });

    // Return to Title action
    ActionDispatcher::register_action(
        "return_to_title",
        [](IGame *game, const std::vector<std::string> &params) {
            Logger::info("[ACTION] Return to Title triggered");
            auto &g = static_cast<Game &>(*game);
            std::string title_path = udjourney::coreutils::get_assets_path(
                "levels/title_screen.json");
            g.m_state = GameState::TITLE;
            g.load_and_apply_scene(title_path);
        });

    // Resume Game action
    ActionDispatcher::register_action(
        "resume_game", [](IGame *game, const std::vector<std::string> &params) {
            Logger::info("[ACTION] Resume Game");
            auto &g = static_cast<Game &>(*game);
            g.hide_game_menu();
        });

    // Restart Level action
    ActionDispatcher::register_action(
        "restart_level",
        [](IGame *game, const std::vector<std::string> &params) {
            Logger::info("[ACTION] Restart Level");
            auto &g = static_cast<Game &>(*game);
            g.hide_game_menu();
            g.restart_level();
        });
}

void Game::initialize_gameplay() {
    hide_game_menu();
    // Remove all widgets from m_actors
    m_actors.erase(std::remove_if(m_actors.begin(),
                                  m_actors.end(),
                                  [](const std::unique_ptr<IActor> &actor) {
                                      return actor->get_group_id() ==
                                             4;  // Remove widgets
                                  }),
                   m_actors.end());

    // Clear background HUDs
    m_hud_manager.clear_background_huds();

    // Create platforms from scene
    create_platforms_from_scene();

    // Spawn monsters from scene
    create_monsters_from_scene();

    create_player();

    // Create HUD objects from scene
    create_huds_from_scene();

    // Add bonus item
    m_actors.emplace_back(
        std::make_unique<Bonus>(*this, Rectangle{300, 300, 20, 20}));

    // Reset score and camera
    m_score = 0;
    m_rect.y = 0;
    m_last_checkpoint = Vector2{320, 240};
}

void Game::create_player() {
    // Spawn player at scene-defined location or default position
    Vector2 player_spawn_pos{320, 240};  // Default position
    if (m_current_scene) {
        auto spawn_data = m_current_scene->get_player_spawn();
        player_spawn_pos = udjourney::scene::Scene::tile_to_world_pos(
            spawn_data.tile_x, spawn_data.tile_y);
    }

    // Get physics config from scene
    const auto &physics_config = m_current_scene
                                     ? m_current_scene->get_physics_config()
                                     : scene::LevelPhysicsConfig{};

    m_player = std::make_unique<Player>(
        *this,
        Rectangle{player_spawn_pos.x, player_spawn_pos.y, 20, 20},
        m_event_dispatcher,
        create_player_animation_controller(),
        physics_config);
    m_player->add_observer(static_cast<IObserver *>(this));
    auto shoot_command =
        std::make_unique<udjourney::commands::CallbackCommand>([this]() {
            if (m_player) {
                const udjourney::ProjectilePreset *preset =
                    m_player->get_current_projectile_preset();
                if (preset) {
                    auto projectile = std::make_unique<udjourney::Projectile>(
                        *this,
                        *preset,
                        m_player->get_shoot_position(),
                        m_player->get_shoot_direction());
                    add_actor(std::move(projectile));
                    m_player->reset_shoot_cooldown();
                    udj::core::Logger::info("Projectile spawned!");
                } else {
                    udj::core::Logger::warning("No projectile preset found!");
                }
            }
        });
    m_player->add_command("shoot_projectile", std::move(shoot_command));

    // Load projectile presets
    m_player->load_projectile_presets("projectiles.json");
    m_player->set_current_projectile("bullet");
}
}  // namespace udjourney
