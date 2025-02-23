#include "Player.hpp"

#include "raylib/raymath.h"
#include "raylib/rlgl.h"

Player::Player(IGame &game, Rectangle r) : IActor(game), r(r) {
                                           };

void Player::draw() const
{
    DrawRectangleRec(r, RED);
}

void Player::update(float delta)
{
    if (IsKeyDown(KEY_LEFT))
    {
        r.x -= 1;
    }
    if (IsKeyDown(KEY_RIGHT))
    {
        r.x += 1;
    }
    if (IsKeyDown(KEY_UP))
    {
        r.y -= 1;
    }
    if (IsKeyDown(KEY_DOWN))
    {
        r.y += 1;
    }
}

void Player::process_input(cont_state_t *cont)
{
    // Do nothing
}