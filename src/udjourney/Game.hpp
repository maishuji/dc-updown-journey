#ifndef GAME_HPP
#define GAME_HPP

#include <memory> // unique_ptr
#include <vector> // vector

#include <kos.h>           // maple_device_t, cont_state_t
#include <raylib/raylib.h> // Rectangle

#include "IGame.hpp"
#include "IActor.hpp"
#include "IObserver.hpp"

enum class GameState : uint8_t
{
    TITLE,
    PLAY,
    PAUSE,
    GAMEOVER
};

class Game : public IGame, public IObserver
{
public:
    Game(int w, int h);
    void run() override;
    void update() override;
    void add_actor(std::unique_ptr<IActor> actor) override;
    void remove_actor(IActor &actor) override;
    void process_input(cont_state_t *cont);
    void on_notify(const std::string &event);

private:
    void draw() const;
    std::vector<std::unique_ptr<IActor>> m_pending_actors;
    std::vector<std::unique_ptr<IActor>> m_actors;
    std::vector<std::unique_ptr<IActor>> m_dead_actors;
    bool m_updating_actors = false;
    GameState m_state = GameState::TITLE;
    double last_update_time = 0.0;
};

#endif // GAME_HPP