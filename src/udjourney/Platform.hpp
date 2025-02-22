#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include <raylib/rlgl.h>

#include "Actor.hpp"

class Platform : public IActor
{
public:
    Platform(Rectangle r);
    void draw() const override;
    void update(float delta) override;

private:
    Rectangle r;
};

#endif // PLATFORM_HPP