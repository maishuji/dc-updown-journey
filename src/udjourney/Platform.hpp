#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include <kos.h> // maple_device_t, cont_state_t
#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include <raylib/rlgl.h>

#include "IActor.hpp"
#include "IGame.hpp"

class Platform : public IActor
{
public:
    Platform(IGame &game, Rectangle r);
    void draw() const override;
    void update(float delta) override;
    void process_input(cont_state_t *t) override;

private:
    Rectangle r;
};

#endif // PLATFORM_HPP