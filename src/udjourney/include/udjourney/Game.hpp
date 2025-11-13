// Copyright 2025 Quentin Cartier

#pragma once
#ifdef PLATFORM_DREAMCAST
#include <kos.h>  // maple_device_t, cont_state_t
#endif
#include <raylib/raylib.h>  // Rectangle

#include <memory>
#include <string>
#include <vector>

#include "udjourney/ScoreHistory.hpp"
#include "udjourney/core/events/EventDispatcher.hpp"
#include "udjourney/interfaces/IActor.hpp"
#include "udjourney/interfaces/IGame.hpp"
#include "udjourney/interfaces/IObserver.hpp"
#include "udjourney/managers/BonusManager.hpp"
#include "udjourney/managers/HUDManager.hpp"
#include "udjourney/scene/Scene.hpp"
#include "udjourney/Player.hpp"

enum class GameState : uint8_t { TITLE, PLAY, PAUSE, GAMEOVER, WIN };

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

    // Scene management
    bool load_scene(const std::string &filename);
    void create_platforms_from_scene();
    void create_monsters_from_scene();
    void restart_level();

    // Level selection
    void show_level_select_menu();
    void hide_level_select_menu();
    void on_level_selected(const std::string &level_path);
    void on_level_select_cancelled();

 private:
    void draw() const;
    void draw_finish_line_() const;
    bool should_continue_scrolling_() const noexcept;

    std::unique_ptr<Player> m_player;  // Player is now a member
    std::vector<std::unique_ptr<IActor>> m_pending_actors;
    std::vector<std::unique_ptr<IActor>> m_actors;
    std::vector<std::unique_ptr<IActor>> m_dead_actors;
    bool m_updating_actors = false;
    GameState m_state = GameState::TITLE;
    Rectangle m_rect;
    double m_last_update_time = 0.0;
    BonusManager m_bonus_manager;
    ScoreHistory<int64_t> m_score_history;
    int m_score = 0;
    HUDManager m_hud_manager;
    udjourney::core::events::EventDispatcher m_event_dispatcher;
    std::unique_ptr<udjourney::scene::Scene> m_current_scene;
    float m_level_height = 0.0f;  // Track level height for win condition
    mutable Vector2 m_last_checkpoint{320, 240};  // Last checkpoint position
    bool m_showing_level_select = false;  // Track if level select menu is shown
};
