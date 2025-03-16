// Copyright 2025 Quentin Cartier

#include "udjourney/Bonus.hpp"

Bonus::Bonus(const IGame &game, Rectangle r) : IActor(game), r(r) {}

void Bonus::draw() const { DrawRectangleRec(r, YELLOW); }

void Bonus::update(float delta) {
    // r.y -= 1;

    if (r.y < 0) {
        r.y = 480;
    }
}

void Bonus::process_input(cont_state_t *t) {
    // Do nothing
}
