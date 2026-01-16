// Copyright 2025 Quentin Cartier

#pragma once
#ifdef PLATFORM_DREAMCAST
#include <kos.h>  // maple_device_t, cont_state_t
#endif
#include <raylib/raylib.h>  // Rectangle

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "udjourney/ScoreHistory.hpp"
#include "udjourney/core/events/EventDispatcher.hpp"
#include "udjourney/interfaces/IActor.hpp"
#include "udjourney/interfaces/IGame.hpp"
#include "udjourney/interfaces/IObserver.hpp"
#include "udjourney/managers/BonusManager.hpp"
#include "udjourney/managers/BackgroundManager.hpp"
#include "udjourney/managers/HUDManager.hpp"
#include "udjourney/managers/LevelSelectManager.hpp"
#include "udjourney/managers/MenuManager.hpp"
#include "udjourney/managers/ParticleManager.hpp"
#include "udjourney/loaders/ParticlePresetLoader.hpp"
#include "udjourney/loaders/ParticlePresetLoader.hpp"
#include "udjourney/render/IStateRenderer.hpp"
#include "udjourney/scene/Scene.hpp"
#include "udjourney/Player.hpp"
#include "udjourney/WorldBounds.hpp"
#include "udjourney/hud/scene/IHUD.hpp"

namespace udjourney {

enum class GameState : uint8_t { TITLE, PLAY, PAUSE, GAMEOVER, WIN };

struct DashHud {
    int16_t dashable = 1;
};

class Game : public IGame, public IObserver {
 public:
    Game(int iWidth, int iHeight);
    void run() override;
    void update() override;
    void add_actor(std::unique_ptr<IActor> actor) override;
    void remove_actor(IActor *actor) override;
    void process_input() override;
    void on_notify(const std::string &event) override;
    void on_checkpoint_reached(float x, float y) const override;
    [[nodiscard]] Rectangle get_rectangle() const override { return m_rect; }
    [[nodiscard]] Player *get_player() const override;
    [[nodiscard]] const udjourney::WorldBounds &get_world_bounds()
        const override {
        return m_world_bounds;
    }

    // Score access for HUDs
    [[nodiscard]] int get_score() const {
        // Return final score on game over/win screens for display
        if (m_state == GameState::GAMEOVER || m_state == GameState::WIN) {
            return m_final_score;
        }
        return m_score;
    }

    // Public accessors for state renderers
    const std::vector<std::unique_ptr<IActor>> &get_actors() const {
        return m_actors;
    }
    const udjourney::scene::Scene *get_current_scene() const {
        return m_current_scene.get();
    }
    float get_level_height() const { return m_level_height; }
    const struct DashHud &get_dash_hud() const;

    // Public draw helpers for renderers
    void draw_backgrounds() const { draw_backgrounds_(); }
    void draw_huds() const { draw_huds_(); }
    void draw_particles() const {
        Rectangle rect = get_rectangle();
        m_particle_manager.draw(Vector2{rect.x, rect.y});
    }

    // Scene management
    bool load_scene(const std::string &filename);

    enum class SceneApplyMode : uint8_t { Auto, Gameplay, UiScreen };

    void apply_current_scene(SceneApplyMode mode = SceneApplyMode::Auto);
    bool load_and_apply_scene(const std::string &filename);

    void create_player();

    void restart_level();

    // HUD manager access for LevelSelectManager
    HUDManager &get_hud_manager() { return m_hud_manager; }

    // Particle manager access
    ParticleManager &get_particle_manager() { return m_particle_manager; }

 private:
    void register_core_event_handlers_();

    // Level selection helper methods
    void show_level_select_menu();
    void hide_level_select_menu();
    void on_level_selected(const std::string &level_path);
    void on_level_select_cancelled();

    void clear_scene();

    // Game menu helper methods
    void show_game_menu();
    void hide_game_menu();

    void draw() const;
    void draw_backgrounds_() const;
    void draw_finish_line_() const;
    void draw_huds_() const;
    bool should_continue_scrolling_() const noexcept;
    void attack_nearby_monsters();

    // Widget and scene management
    void create_platforms_from_scene();
    void create_monsters_from_scene();
    void create_huds_from_scene();
    void register_menu_actions();
    void initialize_gameplay();
    void init_state_renderers_();

    // Deferred notification processing
    void process_pending_notifications();
    void process_notification_immediate(const std::string &event);

    std::unique_ptr<Player> m_player;  // Player is now a member
    std::vector<std::unique_ptr<IActor>> m_pending_actors;
    std::vector<std::unique_ptr<IActor>> m_actors;
    std::vector<std::string>
        m_pending_notifications;  // Queued notifications for safe processing
    std::vector<std::unique_ptr<IActor>> m_dead_actors;
    bool m_updating_actors = false;
    GameState m_state = GameState::TITLE;
    Rectangle m_rect;
    double m_last_update_time = 0.0;
    BonusManager m_bonus_manager;
    ScoreHistory<int64_t> m_score_history;
    int m_score = 0;
    int m_final_score = 0;  // Score to display on game over/win screens
    HUDManager m_hud_manager;
    BackgroundManager m_background_manager;
    ParticleManager m_particle_manager;
    ParticlePresetLoader m_particle_preset_loader;
    udjourney::core::events::EventDispatcher m_event_dispatcher;
    std::unique_ptr<udjourney::scene::Scene> m_current_scene;
    float m_level_height = 0.0f;  // Track level height for win condition
    mutable Vector2 m_last_checkpoint{320, 240};  // Last checkpoint position
    udjourney::managers::LevelSelectManager m_level_select_manager{
        *this};  // Level selection manager
    udjourney::managers::MenuManager m_menu_manager{
        *this};                             // Game menu manager
    udjourney::WorldBounds m_world_bounds;  // World boundary management
    mutable std::map<std::string, Texture2D>
        m_hud_textures;               // Cache for HUD textures
    int m_selected_widget_index = 0;  // Currently focused widget
    int m_frames_since_scene_load =
        0;  // Prevent immediate input after scene transition
    std::vector<std::unique_ptr<udjourney::hud::scene::IHUD>>
        m_scene_huds;  // Scene-based HUD elements

    // State-specific renderers
    std::unordered_map<GameState, std::unique_ptr<IStateRenderer>>
        m_state_renderers;
    std::string m_current_scene_filename;  // Track current scene for restart
    std::string m_last_gameplay_level_filename;  // Track last gameplay level
                                                 // for restart from game over
};
}  // namespace udjourney
