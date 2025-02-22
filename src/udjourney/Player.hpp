#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <raylib/raylib.h>

#include "Actor.hpp"

class Player : public IActor
{
public:
    Player(Rectangle r);
    void draw() const override;
    void update(float delta) override;

private:
    Rectangle r;
};

#endif // PLAYER_HPP