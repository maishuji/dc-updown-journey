// Copyright 2025 Quentin Cartier

#include "udjourney/Platform.hpp"

#include "udjourney/IGame.hpp"

Platform::Platform(const IGame &game, Rectangle r) : IActor(game), r(r) {}

void Platform::draw() const {
    auto rect = r;
    auto &game = get_game();
    // Convert to screen coordinates
    rect.x -= game.get_rectangle().x;
    rect.y -= game.get_rectangle().y;

    DrawRectangleRec(rect, BLUE);
}

void Platform::update(float delta) {
    // Do nothing
}

void Platform::process_input(cont_state_t *t) {
    // Do nothing
}
