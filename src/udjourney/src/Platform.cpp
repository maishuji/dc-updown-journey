// Copyright 2025 Quentin Cartier

#include "udjourney/Platform.hpp"

#include "udjourney/IGame.hpp"

Platform::Platform(const IGame &game, Rectangle r, Color c, bool y_repeated) :
    IActor(game), r(r), color(c), y_repeated(y_repeated) {}

void Platform::draw() const {
    auto rect = r;
    auto &game = get_game();
    // Convert to screen coordinates
    rect.x -= game.get_rectangle().x;
    rect.y -= game.get_rectangle().y;

    DrawRectangleRec(rect, color);
}

void Platform::update(float delta) {
    const auto &gameRect = get_game().get_rectangle();
    // Mark the platform as consummed if it goes out of the screen
    if (r.y + r.height < gameRect.y) {
        set_state(ActorState::CONSUMED);
    }
}

void Platform::process_input(cont_state_t *t) {
    // Do nothing
}
