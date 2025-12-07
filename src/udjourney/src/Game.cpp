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

#include "udjourney/Bonus.hpp"
#include <udj-core/CoreUtils.hpp>
#include "udjourney/Monster.hpp"
#include "udjourney/Player.hpp"
#include "udjourney/Projectile.hpp"
#include "udjourney/AnimSpriteController.hpp"
#include "udjourney/SpriteAnim.hpp"
#include "udjourney/ScoreHistory.hpp"
#include <udj-core/Logger.hpp>
#include "udjourney/hud/DialogBoxHUD.hpp"
#include "udjourney/hud/HUDComponent.hpp"
#include "udjourney/ActionDispatcher.hpp"
#include "udjourney/widgets/IWidget.hpp"
#include "udjourney/widgets/WidgetFactory.hpp"
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
#include "udjourney/components/HealthComponent.hpp"
#include "udjourney/platform/features/SpikeFeature.hpp"
#include "udjourney/platform/reuse_strategies/NoReuseStrategy.hpp"
#include "udjourney/platform/reuse_strategies/PlatformReuseStrategy.hpp"
#include "udjourney/platform/reuse_strategies/RandomizePositionStrategy.hpp"

using udj::core::filesystem::file_exists;
using udj::core::filesystem::get_assets_path;

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

    // Register menu action callbacks
    register_menu_actions();

    // Load title screen scene
    if (!load_scene(udjourney::coreutils::get_assets_path(
            "levels/title_screen.json"))) {
        std::cout << "ERROR: Could not load title_screen.json" << std::endl;
    }
}

void Game::run() {
#ifndef PLATFORM_DREAMCAST
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
#endif
    InitWindow(static_cast<int>(m_rect.width),
               static_cast<int>(m_rect.height),
               "Up-Down Journey");

    // Only initialize gameplay if not in TITLE state
    if (m_state != GameState::TITLE) {
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

        m_bonus_manager.add_observer(static_cast<IObserver *>(this));

        auto score_hud =
            std::make_unique<ScoreHUD>(Vector2{10, 50}, m_event_dispatcher);
        m_hud_manager.add_background_hud(std::move(score_hud));
    } else {
        // Load widgets from title screen scene
        load_widgets_from_scene();
    }

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

    // Pause / Unpause the game
    auto start_pressed = input_mapping.pressed_start();
    if (start_pressed) {
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

void draw_pause_() {
    DrawText(" -- PAUSE -- \n", 10, 10, 20, RED);
    DrawText("Press START to resume\n", 300, 40, 20, WHITE);
    DrawText("Press L to load level\n", 300, 70, 20, WHITE);
    DrawText("Press B button to quit\n", 300, 100, 20, RED);
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

void Game::draw_backgrounds() const {
    if (!m_current_scene) {
        return;
    }

    const auto &layers = m_current_scene->get_background_layers();
    if (layers.empty()) {
        return;
    }

    // Get camera position for parallax effect
    // Only vertical scrolling in this game - horizontal camera is fixed at 0
    // For UI screens (TITLE, GAMEOVER, WIN), use scrolling background offset
    // For gameplay, use m_rect.y for vertical scroll offset
    float scroll_y =
        (m_state == GameState::TITLE || m_state == GameState::GAMEOVER ||
         m_state == GameState::WIN)
            ? m_ui_background_scroll_y
            : m_rect.y;
    Vector2 camera_pos = {0, scroll_y};

    // Sort layers by depth (lower depth = further back)
    std::vector<const udjourney::scene::BackgroundLayerData *> sorted_layers;
    for (const auto &layer : layers) {
        sorted_layers.push_back(&layer);
    }
    std::sort(sorted_layers.begin(),
              sorted_layers.end(),
              [](const auto *a, const auto *b) { return a->depth < b->depth; });

    // Draw each layer
    for (const auto *layer : sorted_layers) {
        // Calculate parallax offset
        // parallax_factor 0.0 = follows camera fully (moves with scene)
        // parallax_factor 1.0 = static (doesn't move, stays fixed on screen)
        float parallax_offset_x =
            camera_pos.x * (1.0f - layer->parallax_factor);
        float parallax_offset_y =
            camera_pos.y * (1.0f - layer->parallax_factor);

        // Note: texture_file at layer level is not used for rendering
        // Background rendering is done through objects in the layer

        // Draw background objects in this layer
        for (const auto &obj : layer->objects) {
            // Load sprite sheet texture if not cached
            if (!obj.sprite_sheet.empty()) {
                if (m_background_textures.find(obj.sprite_sheet) ==
                    m_background_textures.end()) {
                    std::string texture_path =
                        std::string(ASSETS_BASE_PATH) + obj.sprite_sheet;
                    Texture2D tex = LoadTexture(texture_path.c_str());
                    if (tex.id > 0) {
                        m_background_textures[obj.sprite_sheet] = tex;
                        udj::core::Logger::info(
                            "Loaded background sprite sheet: %", texture_path);
                    } else {
                        udj::core::Logger::error(
                            "Failed to load background sprite sheet: %",
                            texture_path);
                    }
                }

                if (m_background_textures[obj.sprite_sheet].id > 0) {
                    const auto &tex = m_background_textures[obj.sprite_sheet];

                    // Calculate tile source rectangle (UV coordinates)
                    Rectangle source = {
                        static_cast<float>(obj.tile_col * obj.tile_size),
                        static_cast<float>(obj.tile_row * obj.tile_size),
                        static_cast<float>(obj.tile_size),
                        static_cast<float>(obj.tile_size)};

                    // Convert world coordinates to screen coordinates with
                    // parallax Object positions are in world coordinates,
                    // subtract game scroll with parallax factor
                    // Background is centered: x=320 in background = screen
                    // center
                    constexpr float BG_CENTER_OFFSET = 320.0f;
                    float screen_x =
                        obj.x - parallax_offset_x - BG_CENTER_OFFSET;

                    float screen_y;
                    // For UI screens with auto-scroll enabled, apply scroll to
                    // objects
                    if (m_current_scene->get_type() ==
                            udjourney::scene::SceneType::UI_SCREEN &&
                        layer->auto_scroll_enabled) {
                        // Apply scroll offset to objects for auto-scrolling
                        // layers
                        float base_y = obj.y - m_ui_background_scroll_y;

                        // If repeat is enabled, wrap the objects vertically
                        if (layer->repeat) {
                            // Calculate wrap height based on object positions
                            // in the layer Use 1280.0f as the wrap boundary
                            // (screen height * 2)
                            float wrap_height = 1280.0f;

                            // Wrap: when object goes off bottom (base_y >
                            // screen_height), bring it back to top (base_y -
                            // wrap_height)
                            screen_y = fmod(base_y, wrap_height);
                            if (screen_y < -128.0f) {
                                screen_y += wrap_height;
                            }

                            // Draw a second copy for seamless wrapping
                            Rectangle dest_wrap = {
                                screen_x,
                                screen_y - wrap_height,
                                static_cast<float>(obj.tile_size * obj.scale),
                                static_cast<float>(obj.tile_size * obj.scale)};
                            DrawTexturePro(tex,
                                           source,
                                           dest_wrap,
                                           {0, 0},
                                           obj.rotation,
                                           WHITE);
                        } else {
                            screen_y = base_y;
                        }
                    } else if (m_current_scene->get_type() ==
                               udjourney::scene::SceneType::UI_SCREEN) {
                        // Static UI screen objects (no auto-scroll)
                        screen_y = obj.y;
                    } else {
                        // Gameplay levels: apply parallax for scrolling
                        screen_y = obj.y - parallax_offset_y;
                    }

                    // Calculate destination rectangle with scale
                    float scaled_size = obj.tile_size * obj.scale;
                    Rectangle dest = {
                        screen_x, screen_y, scaled_size, scaled_size};

                    // Draw the sprite tile
                    Vector2 origin = {0, 0};
                    DrawTexturePro(
                        tex, source, dest, origin, obj.rotation, WHITE);
                }
            }
        }
    }
}

void Game::draw_fuds_() const {
    if (!m_current_scene) {
        return;
    }

    const auto &fuds = m_current_scene->get_fuds();
    if (fuds.empty()) {
        return;
    }

    // FUDs are drawn in screen space (not affected by camera/scrolling)
    for (const auto &fud : fuds) {
        if (!fud.visible) {
            continue;
        }

        // Calculate anchor position based on screen size
        float anchor_x = 0.0f;
        float anchor_y = 0.0f;

        float screen_width = static_cast<float>(kBaseWidth);
        float screen_height = static_cast<float>(kBaseHeight);

        switch (fud.anchor) {
            case udjourney::scene::FUDAnchor::TopLeft:
                anchor_x = 0;
                anchor_y = 0;
                break;
            case udjourney::scene::FUDAnchor::TopCenter:
                anchor_x = screen_width / 2;
                anchor_y = 0;
                break;
            case udjourney::scene::FUDAnchor::TopRight:
                anchor_x = screen_width;
                anchor_y = 0;
                break;
            case udjourney::scene::FUDAnchor::MiddleLeft:
                anchor_x = 0;
                anchor_y = screen_height / 2;
                break;
            case udjourney::scene::FUDAnchor::MiddleCenter:
                anchor_x = screen_width / 2;
                anchor_y = screen_height / 2;
                break;
            case udjourney::scene::FUDAnchor::MiddleRight:
                anchor_x = screen_width;
                anchor_y = screen_height / 2;
                break;
            case udjourney::scene::FUDAnchor::BottomLeft:
                anchor_x = 0;
                anchor_y = screen_height;
                break;
            case udjourney::scene::FUDAnchor::BottomCenter:
                anchor_x = screen_width / 2;
                anchor_y = screen_height;
                break;
            case udjourney::scene::FUDAnchor::BottomRight:
                anchor_x = screen_width;
                anchor_y = screen_height;
                break;
        }

        // Calculate final FUD position
        float fud_x = anchor_x + fud.offset_x;
        float fud_y = anchor_y + fud.offset_y;

        // Draw background image/sprite if specified
        if (!fud.background_sheet.empty()) {
            // Load sprite sheet if not cached
            if (m_fud_textures.find(fud.background_sheet) ==
                m_fud_textures.end()) {
                std::string texture_path =
                    std::string(ASSETS_BASE_PATH) + fud.background_sheet;
                Texture2D tex = LoadTexture(texture_path.c_str());
                if (tex.id > 0) {
                    m_fud_textures[fud.background_sheet] = tex;
                    udj::core::Logger::info("Loaded FUD sprite sheet: %",
                                            texture_path);
                } else {
                    udj::core::Logger::error(
                        "Failed to load FUD sprite sheet: %", texture_path);
                }
            }

            // Draw sprite from sheet
            if (m_fud_textures[fud.background_sheet].id > 0) {
                const auto &tex = m_fud_textures[fud.background_sheet];

                // Calculate source rectangle in sprite sheet
                Rectangle source = {
                    static_cast<float>(fud.background_tile_col *
                                       fud.background_tile_size),
                    static_cast<float>(fud.background_tile_row *
                                       fud.background_tile_size),
                    static_cast<float>(fud.background_tile_width *
                                       fud.background_tile_size),
                    static_cast<float>(fud.background_tile_height *
                                       fud.background_tile_size)};

                Rectangle dest = {fud_x, fud_y, fud.size_x, fud.size_y};
                DrawTexturePro(tex, source, dest, {0, 0}, 0.0f, WHITE);
            }
        } else if (!fud.background_image.empty()) {
            // Legacy: single image file
            if (m_fud_textures.find(fud.background_image) ==
                m_fud_textures.end()) {
                std::string texture_path =
                    std::string(ASSETS_BASE_PATH) + fud.background_image;
                Texture2D tex = LoadTexture(texture_path.c_str());
                if (tex.id > 0) {
                    m_fud_textures[fud.background_image] = tex;
                    udj::core::Logger::info("Loaded FUD texture: %",
                                            texture_path);
                } else {
                    udj::core::Logger::error("Failed to load FUD texture: %",
                                             texture_path);
                }
            }

            if (m_fud_textures[fud.background_image].id > 0) {
                const auto &tex = m_fud_textures[fud.background_image];
                Rectangle source = {0,
                                    0,
                                    static_cast<float>(tex.width),
                                    static_cast<float>(tex.height)};
                Rectangle dest = {fud_x, fud_y, fud.size_x, fud.size_y};
                DrawTexturePro(tex, source, dest, {0, 0}, 0.0f, WHITE);
            }
        }

        // Draw FUD based on type
        if (fud.type_id == "heart_health") {
            // Draw health as hearts (Zelda-style)
            // Parse properties
            int max_hearts = 3;
            int heart_spacing = 32;
            bool show_empty = true;
            int current_half_hearts = 6;  // Track half-hearts

            // Get current health from player's HealthComponent
            if (m_player) {
                if (auto *health = m_player->get_component<HealthComponent>()) {
                    // Health is stored as half-hearts (2 per full heart)
                    current_half_hearts = health->get_health();
                    max_hearts = (health->get_max_health() + 1) / 2;

                    // Debug output
                    static int last_health = -1;
                    if (current_half_hearts != last_health) {
                        std::cout
                            << "FUD Drawing - Health: " << current_half_hearts
                            << "/" << health->get_max_health() << std::endl;
                        last_health = current_half_hearts;
                    }
                } else {
                    std::cout << "WARNING: Player has no HealthComponent!"
                              << std::endl;
                }
            } else {
                std::cout << "WARNING: No player found for FUD health display!"
                          << std::endl;
            }

            // Heart sprite configuration
            std::string heart_sheet = "ui/ui_elements.png";
            int heart_tile_size = 32;
            int full_col = 0, full_row = 3;
            int half_col = 3, half_row = 3;
            int empty_col = 4, empty_row = 3;

            try {
                if (fud.properties.count("max_hearts")) {
                    max_hearts = std::stoi(fud.properties.at("max_hearts"));
                }
                // NOTE: current_hearts is NOT loaded from JSON - it's read from
                // Player's HealthComponent Removed the property parsing that
                // was overwriting the player's health value
                if (fud.properties.count("heart_spacing")) {
                    heart_spacing =
                        std::stoi(fud.properties.at("heart_spacing"));
                }
                if (fud.properties.count("show_empty_hearts")) {
                    show_empty =
                        (fud.properties.at("show_empty_hearts") == "true");
                }

                // Parse heart sprite configurations from JSON
                if (fud.properties.count("heart_full_sprite")) {
                    try {
                        auto sprite_json = nlohmann::json::parse(
                            fud.properties.at("heart_full_sprite"));
                        heart_sheet = sprite_json.value("sheet", heart_sheet);
                        heart_tile_size =
                            sprite_json.value("tile_size", heart_tile_size);
                        full_col = sprite_json.value("tile_col", full_col);
                        full_row = sprite_json.value("tile_row", full_row);
                    } catch (...) {
                    }
                }
                if (fud.properties.count("heart_empty_sprite")) {
                    try {
                        auto sprite_json = nlohmann::json::parse(
                            fud.properties.at("heart_empty_sprite"));
                        empty_col = sprite_json.value("tile_col", empty_col);
                        empty_row = sprite_json.value("tile_row", empty_row);
                    } catch (...) {
                    }
                }
            } catch (...) {
                // Use defaults if parsing fails
            }

            // Load heart sprite sheet
            if (m_fud_textures.find(heart_sheet) == m_fud_textures.end()) {
                std::string texture_path =
                    std::string(ASSETS_BASE_PATH) + heart_sheet;
                Texture2D tex = LoadTexture(texture_path.c_str());
                if (tex.id > 0) {
                    m_fud_textures[heart_sheet] = tex;
                    udj::core::Logger::info("Loaded heart sprite sheet: %",
                                            texture_path);
                } else {
                    udj::core::Logger::error(
                        "Failed to load heart sprite sheet: %", texture_path);
                }
            }

            // Draw hearts horizontally
            if (m_fud_textures[heart_sheet].id > 0) {
                const auto &tex = m_fud_textures[heart_sheet];

                for (int i = 0; i < max_hearts; ++i) {
                    float heart_x = fud_x + (i * heart_spacing);
                    float heart_y = fud_y;

                    // Determine which heart sprite to draw based on half-hearts
                    int half_hearts_for_this_position =
                        current_half_hearts - (i * 2);
                    int sprite_col, sprite_row;

                    if (half_hearts_for_this_position >= 2) {
                        // Full heart
                        sprite_col = full_col;
                        sprite_row = full_row;
                    } else if (half_hearts_for_this_position == 1) {
                        // Half heart
                        sprite_col = half_col;
                        sprite_row = half_row;
                    } else {
                        // Empty heart
                        sprite_col = empty_col;
                        sprite_row = empty_row;
                    }

                    if (!show_empty && half_hearts_for_this_position <= 0) {
                        continue;  // Skip empty hearts if not showing them
                    }

                    // Calculate source rectangle in sprite sheet
                    Rectangle source = {
                        static_cast<float>(sprite_col * heart_tile_size),
                        static_cast<float>(sprite_row * heart_tile_size),
                        static_cast<float>(heart_tile_size),
                        static_cast<float>(heart_tile_size)};

                    Rectangle dest = {heart_x,
                                      heart_y,
                                      static_cast<float>(heart_spacing),
                                      static_cast<float>(heart_spacing)};

                    DrawTexturePro(tex, source, dest, {0, 0}, 0.0f, WHITE);
                }
            } else {
                // Fallback: draw circles if texture not available
                for (int i = 0; i < max_hearts; ++i) {
                    float heart_x = fud_x + (i * heart_spacing);
                    float heart_y = fud_y;

                    int half_hearts_for_this_position =
                        current_half_hearts - (i * 2);
                    bool is_full = (half_hearts_for_this_position >= 2);
                    bool is_half = (half_hearts_for_this_position == 1);

                    if (is_full) {
                        DrawCircle(static_cast<int>(heart_x + 16),
                                   static_cast<int>(heart_y + 16),
                                   12,
                                   RED);
                    } else if (is_half) {
                        DrawCircleSector(Vector2{heart_x + 16, heart_y + 16},
                                         12,
                                         90,
                                         270,
                                         16,
                                         RED);
                        DrawCircleSectorLines(
                            Vector2{heart_x + 16, heart_y + 16},
                            12,
                            90,
                            270,
                            16,
                            RED);
                    } else if (show_empty) {
                        DrawCircle(static_cast<int>(heart_x + 16),
                                   static_cast<int>(heart_y + 16),
                                   12,
                                   ColorAlpha(RED, 0.3f));
                        DrawCircleLines(static_cast<int>(heart_x + 16),
                                        static_cast<int>(heart_y + 16),
                                        12,
                                        RED);
                    }
                }
            }
        } else if (fud.type_id == "healthbar" || fud.type_id == "mana_bar") {
            // Draw a simple health/mana bar
            Color bar_color = fud.type_id == "healthbar" ? RED : BLUE;
            Color bg_color = DARKGRAY;

            // Background
            DrawRectangle(static_cast<int>(fud_x),
                          static_cast<int>(fud_y),
                          static_cast<int>(fud.size_x),
                          static_cast<int>(fud.size_y),
                          bg_color);

            // Get fill percentage from player health for healthbar
            float fill_percent = 0.8f;  // Default for mana_bar or if no health
            if (fud.type_id == "healthbar" && m_player) {
                if (auto *health = m_player->get_component<HealthComponent>()) {
                    fill_percent = health->get_health_percentage();
                }
            }

            // Bar
            DrawRectangle(static_cast<int>(fud_x + 2),
                          static_cast<int>(fud_y + 2),
                          static_cast<int>((fud.size_x - 4) * fill_percent),
                          static_cast<int>(fud.size_y - 4),
                          bar_color);

            // Border
            DrawRectangleLines(static_cast<int>(fud_x),
                               static_cast<int>(fud_y),
                               static_cast<int>(fud.size_x),
                               static_cast<int>(fud.size_y),
                               WHITE);

            // Text
            DrawText(fud.name.c_str(),
                     static_cast<int>(fud_x + 5),
                     static_cast<int>(fud_y + 5),
                     12,
                     WHITE);
        } else if (fud.type_id == "score_display") {
            // Draw score display
            DrawRectangle(static_cast<int>(fud_x),
                          static_cast<int>(fud_y),
                          static_cast<int>(fud.size_x),
                          static_cast<int>(fud.size_y),
                          ColorAlpha(BLACK, 0.5f));

            char score_text[64];
            snprintf(score_text, sizeof(score_text), "Score: %d", m_score);
            DrawText(score_text,
                     static_cast<int>(fud_x + 10),
                     static_cast<int>(fud_y + 10),
                     20,
                     WHITE);
        } else if (fud.type_id == "timer") {
            // Draw timer
            DrawRectangle(static_cast<int>(fud_x),
                          static_cast<int>(fud_y),
                          static_cast<int>(fud.size_x),
                          static_cast<int>(fud.size_y),
                          ColorAlpha(BLACK, 0.5f));

            // Simple countdown timer (demo - would need actual timer logic)
            int minutes = 3;
            int seconds = 0;
            char timer_text[32];
            snprintf(
                timer_text, sizeof(timer_text), "%02d:%02d", minutes, seconds);
            DrawText(timer_text,
                     static_cast<int>(fud_x + 10),
                     static_cast<int>(fud_y + 5),
                     24,
                     YELLOW);
        } else {
            // Generic FUD - just draw a labeled box
            DrawRectangle(static_cast<int>(fud_x),
                          static_cast<int>(fud_y),
                          static_cast<int>(fud.size_x),
                          static_cast<int>(fud.size_y),
                          ColorAlpha(YELLOW, 0.3f));
            DrawRectangleLines(static_cast<int>(fud_x),
                               static_cast<int>(fud_y),
                               static_cast<int>(fud.size_x),
                               static_cast<int>(fud.size_y),
                               YELLOW);
            DrawText(fud.name.c_str(),
                     static_cast<int>(fud_x + 5),
                     static_cast<int>(fud_y + 5),
                     12,
                     WHITE);
        }

        // Draw foreground image/sprite if specified (overlays on top)
        if (!fud.foreground_sheet.empty()) {
            // Load sprite sheet if not cached
            if (m_fud_textures.find(fud.foreground_sheet) ==
                m_fud_textures.end()) {
                std::string texture_path =
                    std::string(ASSETS_BASE_PATH) + fud.foreground_sheet;
                Texture2D tex = LoadTexture(texture_path.c_str());
                if (tex.id > 0) {
                    m_fud_textures[fud.foreground_sheet] = tex;
                    udj::core::Logger::info("Loaded FUD sprite sheet: %",
                                            texture_path);
                } else {
                    udj::core::Logger::error(
                        "Failed to load FUD sprite sheet: %", texture_path);
                }
            }

            // Draw sprite from sheet
            if (m_fud_textures[fud.foreground_sheet].id > 0) {
                const auto &tex = m_fud_textures[fud.foreground_sheet];

                // Calculate source rectangle in sprite sheet
                Rectangle source = {
                    static_cast<float>(fud.foreground_tile_col *
                                       fud.foreground_tile_size),
                    static_cast<float>(fud.foreground_tile_row *
                                       fud.foreground_tile_size),
                    static_cast<float>(fud.foreground_tile_width *
                                       fud.foreground_tile_size),
                    static_cast<float>(fud.foreground_tile_height *
                                       fud.foreground_tile_size)};

                Rectangle dest = {fud_x, fud_y, fud.size_x, fud.size_y};
                DrawTexturePro(tex, source, dest, {0, 0}, 0.0f, WHITE);
            }
        } else if (!fud.foreground_image.empty()) {
            // Legacy: single image file
            if (m_fud_textures.find(fud.foreground_image) ==
                m_fud_textures.end()) {
                std::string texture_path =
                    std::string(ASSETS_BASE_PATH) + fud.foreground_image;
                Texture2D tex = LoadTexture(texture_path.c_str());
                if (tex.id > 0) {
                    m_fud_textures[fud.foreground_image] = tex;
                    udj::core::Logger::info("Loaded FUD texture: %",
                                            texture_path);
                } else {
                    udj::core::Logger::error("Failed to load FUD texture: %",
                                             texture_path);
                }
            }

            if (m_fud_textures[fud.foreground_image].id > 0) {
                const auto &tex = m_fud_textures[fud.foreground_image];
                Rectangle source = {0,
                                    0,
                                    static_cast<float>(tex.width),
                                    static_cast<float>(tex.height)};
                Rectangle dest = {fud_x, fud_y, fud.size_x, fud.size_y};
                DrawTexturePro(tex, source, dest, {0, 0}, 0.0f, WHITE);
            }
        }
    }
}

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
            // Draw scrolling backgrounds
            draw_backgrounds();

            // Draw widgets (menu buttons)
            for (const auto &actor : m_actors) {
                if (actor->get_group_id() == 4) {  // Widget group ID
                    actor->draw();
                }
            }
            break;
        case GameState::PLAY: {
            // Draw backgrounds first (behind everything)
            draw_backgrounds();

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
            // Draw the game world in background
            draw_backgrounds();

            for (const auto &actor : m_actors) {
                actor->draw();
            }
            if (m_player) {
                m_player->draw();
            }

            // Draw finish line for level-based gameplay
            if (m_current_scene && m_level_height > 0) {
                draw_finish_line_();
            }

            // Draw pause overlay on top
            draw_pause_();
            break;
        case GameState::GAMEOVER:
            // Draw scrolling backgrounds
            draw_backgrounds();

            // Draw FUDs (game over message and score)
            if (m_current_scene) {
                draw_fuds_();
            }

            // Draw widgets (menu buttons)
            for (const auto &actor : m_actors) {
                if (actor->get_group_id() == 4) {  // Widget group ID
                    actor->draw();
                }
            }
            break;
        case GameState::WIN:
            // Draw scrolling backgrounds
            draw_backgrounds();

            // Draw FUDs (victory messages)
            if (m_current_scene) {
                draw_fuds_();
            }

            // Draw widgets (menu buttons)
            for (const auto &actor : m_actors) {
                if (actor->get_group_id() == 4) {  // Widget group ID
                    actor->draw();
                }
            }
            break;
    }
    m_hud_manager.draw();

    // Draw FUDs (Fixed UI Displays) from current scene
    // Skip for states that handle their own FUD/widget drawing
    if (m_current_scene && m_state != GameState::TITLE &&
        m_state != GameState::GAMEOVER && m_state != GameState::WIN) {
        draw_fuds_();
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
        m_current_scene->get_type() == udjourney::scene::SceneType::UI_SCREEN) {
        // Use per-layer auto-scroll settings from scene data
        const auto &layers = m_current_scene->get_background_layers();
        // Check all layers with auto-scroll enabled
        float default_scroll_speed = 0.0f;
        float max_scroll_limit = 0.0f;
        bool should_clamp = false;

        for (const auto &layer : layers) {
            if (layer.auto_scroll_enabled) {
                // Use first non-zero scroll speed found
                if (default_scroll_speed == 0.0f &&
                    layer.scroll_speed_y != 0.0f) {
                    default_scroll_speed = layer.scroll_speed_y;
                }

                // Check if we should clamp scrolling for layers without repeat
                if (!layer.repeat && !layer.objects.empty()) {
                    should_clamp = true;
                    // Calculate max Y position from objects in this layer
                    // The layer stops when bottom of layer reaches bottom of
                    // screen
                    float max_y = 0.0f;
                    for (const auto &obj : layer.objects) {
                        float obj_bottom = obj.y + (obj.tile_size * obj.scale);
                        max_y = std::max(max_y, obj_bottom);
                    }
                    // Scroll stops when bottom of content reaches bottom of
                    // screen (kBaseHeight)
                    if (max_y > static_cast<float>(kBaseHeight)) {
                        float scroll_limit =
                            max_y - static_cast<float>(kBaseHeight);
                        max_scroll_limit =
                            std::max(max_scroll_limit, scroll_limit);
                    }
                }
            }
        }

        // If no explicit scroll speed, use default for backward compatibility
        if (default_scroll_speed == 0.0f) {
            // Check if any layer has auto-scroll enabled (legacy mode)
            for (const auto &layer : layers) {
                if (layer.auto_scroll_enabled) {
                    default_scroll_speed = 30.0f;
                    break;
                }
            }
        }

        // Update scroll position
        m_ui_background_scroll_y += default_scroll_speed * GetFrameTime();

        // Clamp to calculated scroll limit
        if (should_clamp && max_scroll_limit > 0.0f) {
            m_ui_background_scroll_y =
                std::min(m_ui_background_scroll_y, max_scroll_limit);
        }
    }

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
                    m_rect.y += 1;
                }
                for (auto &actor : m_actors) {
                    actor->update(delta);
                }
                m_player->update(delta);
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
                            std::cout << "Projectile hit monster! Damage: "
                                      << projectile->get_damage()
                                      << " Monster ptr: " << monster
                                      << std::endl;

                            if (monster) {
                                std::cout << "Calling monster->take_damage..."
                                          << std::endl;
                                monster->take_damage(static_cast<float>(
                                    projectile->get_damage()));
                                std::cout << "take_damage returned"
                                          << std::endl;
                            } else {
                                std::cout << "ERROR: Monster pointer is null!"
                                          << std::endl;
                            }

                            projectile->destroy();
                            std::cout << "Projectile destroyed after hit"
                                      << std::endl;
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
                    if (player_bottom >= m_level_height * 0.98f) {
                        m_state = GameState::WIN;

                        // Load win screen with widgets
                        std::string win_path =
                            udjourney::coreutils::get_assets_path(
                                "levels/win_screen.json");
                        if (load_scene(win_path)) {
                            // Clean up gameplay objects
                            m_player.reset();
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
                            load_widgets_from_scene();
                            m_rect.y = 0;  // Reset camera
                        }
                    }
                }
            }

            // Only handle collisions if player exists (not in WIN/TITLE state)
            if (m_player) {
                m_player->handle_collision(m_actors);
            }

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

            // Load game over screen with widgets
            std::string gameover_path = udjourney::coreutils::get_assets_path(
                "levels/game_over_screen.json");
            if (load_scene(gameover_path)) {
                // Clean up gameplay objects
                m_player.reset();
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
                load_widgets_from_scene();
                m_rect.y = 0;  // Reset camera
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
                float speed = 50.0f;          // Default speed
                float range = 100.0f;         // Default range
                float initial_offset = 0.0f;  // Default starting offset

                if (platform_data.behavior_params.count("speed")) {
                    speed = platform_data.behavior_params.at("speed") *
                            10.0f;  // Scale for pixels
                }
                if (platform_data.behavior_params.count("range")) {
                    range = platform_data.behavior_params.at("range") *
                            32.0f;  // Convert tiles to pixels
                }
                if (platform_data.behavior_params.count("initial_offset")) {
                    initial_offset =
                        platform_data.behavior_params.at("initial_offset") *
                        32.0f;  // Convert tiles to pixels
                }

                platform->set_behavior(
                    std::make_unique<HorizontalBehaviorStrategy>(
                        speed, range, initial_offset));
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

        // Reset player health to full
        if (auto *health = m_player->get_component<HealthComponent>()) {
            health->heal(health->get_max_health());  // Restore to max health
        }
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

void Game::load_widgets_from_scene() {
    if (!m_current_scene) {
        return;
    }

    // Extract ALL widget FUD elements
    const auto &fuds = m_current_scene->get_fuds();
    for (const auto &fud : fuds) {
        // Try to create widget using factory (supports all widget types)
        auto widget = WidgetFactory::create_from_fud(fud, *this);
        if (widget) {
            m_actors.push_back(std::move(widget));
        }
    }

    m_selected_widget_index = 0;
}

void Game::register_menu_actions() {
    // Start Game action
    ActionDispatcher::register_action(
        "start_game", [](IGame *game, const std::vector<std::string> &params) {
            std::cout << "[ACTION] Start Game triggered" << std::endl;
            auto *g = dynamic_cast<Game *>(game);
            if (g) {
                // Load level1 scene first
                std::string level_path =
                    udjourney::coreutils::get_assets_path("levels/level1.json");
                if (g->load_scene(level_path)) {
                    g->m_state = GameState::PLAY;
                    g->initialize_gameplay();
                }
            }
        });

    // Load Level action (format: "load_level:level_name")
    ActionDispatcher::register_action(
        "load_level", [](IGame *game, const std::vector<std::string> &params) {
            if (params.empty()) return;

            // params[0] is the filename (e.g., "level1.json")
            std::string filename = params[0];
            std::cout << "[ACTION] Load Level: " << filename << std::endl;

            auto *g = dynamic_cast<Game *>(game);
            if (g) {
                std::string level_path =
                    udjourney::coreutils::get_assets_path("levels/" + filename);
                if (g->load_scene(level_path)) {
                    g->m_state = GameState::PLAY;
                    g->initialize_gameplay();
                }
            }
        });

    // Show Level Select action
    ActionDispatcher::register_action(
        "show_level_select",
        [](IGame *game, const std::vector<std::string> &params) {
            std::cout << "[ACTION] ===== Show Level Select triggered ====="
                      << std::endl;
            auto *g = dynamic_cast<Game *>(game);
            if (g) {
                std::string level_select_path =
                    udjourney::coreutils::get_assets_path(
                        "levels/level_select_screen.json");

                // Clean up ALL actors first (including old widgets)
                g->m_player.reset();
                g->m_hud_manager.clear_background_huds();
                g->m_actors.clear();

                // Now load the new scene
                if (g->load_scene(level_select_path)) {
                    g->m_state = GameState::TITLE;

                    // Load level select screen widgets
                    g->load_widgets_from_scene();

                    // Set focus to the ScrollableListWidget (should be the
                    // first selectable widget) Find the index of the
                    // ScrollableListWidget in the selectable widgets
                    std::vector<IWidget *> selectable_widgets;
                    int list_index = -1;
                    for (const auto &actor : g->m_actors) {
                        if (actor && actor->get_group_id() == 4) {
                            IWidget *widget =
                                static_cast<IWidget *>(actor.get());
                            if (widget && widget->is_selectable()) {
                                if (dynamic_cast<ScrollableListWidget *>(
                                        widget)) {
                                    list_index = static_cast<int>(
                                        selectable_widgets.size());
                                }
                                selectable_widgets.push_back(widget);
                            }
                        }
                    }

                    // Focus the list widget (or default to 0 if not found)
                    g->m_selected_widget_index =
                        (list_index >= 0) ? list_index : 0;

                    // Reset frame counter to prevent immediate input
                    g->m_frames_since_scene_load = 0;

                    std::cout << "[ACTION] Level select screen loaded with "
                              << g->m_actors.size()
                              << " actors, focus on widget "
                              << g->m_selected_widget_index << std::endl;
                }
            }
        });

    // Quit Game action
    ActionDispatcher::register_action(
        "quit_game", [](IGame *game, const std::vector<std::string> &params) {
            std::cout << "[ACTION] Quit Game triggered" << std::endl;
#ifndef PLATFORM_DREAMCAST
            CloseWindow();
#endif
        });

    // Show Options action (placeholder)
    ActionDispatcher::register_action(
        "show_options",
        [](IGame *game, const std::vector<std::string> &params) {
            std::cout << "[ACTION] Show Options triggered (not implemented)"
                      << std::endl;
        });

    // Return to Title action
    ActionDispatcher::register_action(
        "return_to_title",
        [](IGame *game, const std::vector<std::string> &params) {
            std::cout << "[ACTION] Return to Title triggered" << std::endl;
            auto *g = dynamic_cast<Game *>(game);
            if (g) {
                std::string title_path = udjourney::coreutils::get_assets_path(
                    "levels/title_screen.json");
                if (g->load_scene(title_path)) {
                    g->m_state = GameState::TITLE;

                    // Clean up gameplay objects
                    g->m_player.reset();
                    g->m_hud_manager.clear_background_huds();

                    // Remove ALL actors (including old widgets)
                    g->m_actors.clear();

                    // Load title screen widgets
                    g->load_widgets_from_scene();
                    g->m_rect.y = 0;                   // Reset camera
                    g->m_selected_widget_index = 0;    // Reset widget selection
                    g->m_frames_since_scene_load = 0;  // Reset frame counter
                }
            }
        });
}

void Game::initialize_gameplay() {
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

    // Load projectile presets
    m_player->load_projectile_presets("projectiles.json");
    m_player->set_current_projectile("bullet");

    // Add bonus item
    m_actors.emplace_back(
        std::make_unique<Bonus>(*this, Rectangle{300, 300, 20, 20}));

    // Reset score and camera
    m_score = 0;
    m_rect.y = 0;
    m_last_checkpoint = Vector2{320, 240};

    // Add score HUD
    auto score_hud =
        std::make_unique<ScoreHUD>(Vector2{10, 50}, m_event_dispatcher);
    m_hud_manager.add_background_hud(std::move(score_hud));
}
