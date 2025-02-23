#ifndef GAME_HPP
#define GAME_HPP

#include <memory> // unique_ptr
#include <vector> // vector

#include <kos.h>           // maple_device_t, cont_state_t
#include <raylib/raylib.h> // Rectangle

#include "IActor.hpp"

enum class GameState : uint8_t
{
    TITLE,
    PLAY,
    PAUSE,
    GAMEOVER
};

class Game
{
public:
    Game(int w, int h);
    void run();
    void update();
    void add_actor(std::unique_ptr<IActor> actor);
    void remove_actor(IActor &actor);
    void process_input(cont_state_t *cont);

private:
    std::vector<std::unique_ptr<IActor>> m_pending_actors;
    std::vector<std::unique_ptr<IActor>> m_actors;
    std::vector<std::unique_ptr<IActor>> m_dead_actors;
    bool m_updating_actors = false;
    GameState m_state = GameState::TITLE;
    double last_update_time = 0.0;
};

#endif // GAME_HPP