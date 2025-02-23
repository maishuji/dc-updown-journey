#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <memory>

#include <raylib/raylib.h>
#include <kos.h> // maple_device_t, cont_state_t

#include "IGame.hpp"
#include "IActor.hpp"

class Player : public IActor
{
public:
    Player(IGame &game, Rectangle r);
    void draw() const override;
    void update(float delta) override;
    void process_input(cont_state_t *cont) override;

private:
    Rectangle r;
};

#endif // PLAYER_HPP