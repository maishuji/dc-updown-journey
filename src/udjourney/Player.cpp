#include "Player.hpp"

#include "raylib/raymath.h"
#include "raylib/rlgl.h"

Player::Player(IGame &game, Rectangle r) : IActor(game), r(r) {};

void Player::draw() const
{
    DrawRectangleRec(r, RED);
}

void Player::update(float delta)
{
}

void Player::process_input(cont_state_t *cont)
{
    if (cont->buttons & CONT_DPAD_LEFT)
    {
        r.x -= 5;
    }
    if (cont->buttons & CONT_DPAD_RIGHT)
    {
        r.x += 5;
    }
    if (cont->buttons & CONT_DPAD_UP)
    {
        r.y -= 5;
    }
    if (cont->buttons & CONT_DPAD_DOWN)
    {
        r.y += 5;
    }
}