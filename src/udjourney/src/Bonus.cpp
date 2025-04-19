// Copyright 2025 Quentin Cartier

#include "udjourney/Bonus.hpp"

#include <kos.h>  // maple_device_t, cont_state_t
#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include <raylib/rlgl.h>


Bonus::Bonus(const IGame &game, Rectangle r) : IActor(game), r(r) {}

void Bonus::draw() const {
    auto rect = r;
    auto &game = get_game();
    // Convert to screen coordinates
    rect.x -= game.get_rectangle().x;
    rect.y -= game.get_rectangle().y;
    DrawRectangleRec(rect, YELLOW);
}

void Bonus::update(float delta) {
    // r.y -= 1;

    if (r.y < 0) {
        r.y = 480;
    }
}

void Bonus::process_input(cont_state_t *t) {
    // Do nothing
}
