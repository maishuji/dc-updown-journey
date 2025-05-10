// Copyright 2025 Quentin Cartier

#ifndef SRC_UDJOURNEY_INCLUDE_UDJOURNEY_GAME_HPP_
#define SRC_UDJOURNEY_INCLUDE_UDJOURNEY_GAME_HPP_
#ifdef PLATFORM_DREAMCAST
#include <kos.h>            // maple_device_t, cont_state_t
#endif
#include <raylib/raylib.h>  // Rectangle

#include <memory>
#include <string>
#include <vector>

#include "udjourney/BonusManager.hpp"
#include "udjourney/IActor.hpp"
#include "udjourney/IGame.hpp"
#include "udjourney/IObserver.hpp"
#include "udjourney/ScoreHistory.hpp"

enum class GameState : uint8_t { TITLE, PLAY, PAUSE, GAMEOVER };

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

 private:
    void draw() const;
    std::vector<std::unique_ptr<IActor>> m_pending_actors;
    std::vector<std::unique_ptr<IActor>> m_actors;
    std::vector<std::unique_ptr<IActor>> m_dead_actors;
    bool m_updating_actors = false;
    GameState m_state = GameState::TITLE;
    Rectangle m_rect;
    double m_last_update_time = 0.0;
    BonusManager m_bonus_manager;
    ScoreHistory<int64_t> m_score_history;
    int64_t m_score = 0;
};
#endif  // SRC_UDJOURNEY_INCLUDE_UDJOURNEY_GAME_HPP_
