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

void Player::resolve_collision(const IActor &platform) noexcept
{
    Rectangle platformRect = platform.get_rectangle();

    if (CheckCollisionRecs(r, platformRect))
    {
        if (r.x < platformRect.x)
        {
            r.x = platformRect.x - r.width;
        }
        else if (r.x > platformRect.x)
        {
            r.x = platformRect.x + platformRect.width;
        }

        if (r.y < platformRect.y)
        {
            r.y = platformRect.y - r.height;
        }
        else if (r.y > platformRect.y)
        {
            r.y = platformRect.y + platformRect.height;
        }
    }
}

void Player::handle_collision(const std::vector<std::unique_ptr<IActor>> &platforms) noexcept
{
    for (const auto &platform : platforms)
    {
        if (check_collision(*platform))
        {
            resolve_collision(*platform);
        }
    }
}