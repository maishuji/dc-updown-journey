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
    [[nodiscard]] Rectangle get_rectangle() const override { return m_rect; }

    // Scene management
    bool load_scene(const std::string &filename);
    void create_platforms_from_scene();
    void restart_level();

 private:
    void draw() const;
    void draw_finish_line_() const;
    bool should_continue_scrolling_() const noexcept;

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
};
