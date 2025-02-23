#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <raylib/raylib.h>

#include "IActor.hpp"

class Player : public IActor
{
public:
    Player(Rectangle r);
    void draw() const override;
    void update(float delta) override;
    void process_input(cont_state_t *cont) override;

private:
    Rectangle r;
};

#endif // PLAYER_HPP