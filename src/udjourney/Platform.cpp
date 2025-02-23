#include "Platform.hpp"

Platform::Platform(Rectangle r) : r(r)
{
}

void Platform::draw() const
{
    DrawRectangleRec(r, BLUE);
}

void Platform::update(float delta)
{
    r.y -= 1;

    if (r.y < 0)
    {
        r.y = 480;
    }
}

void Platform::process_input(cont_state_t *t)
{
    // Do nothing
}
