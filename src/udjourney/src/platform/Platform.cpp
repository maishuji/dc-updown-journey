// Copyright 2025 Quentin Cartier

#include "udjourney/platform/Platform.hpp"

#include "udjourney/IGame.hpp"

Platform::Platform(const IGame &game, Rectangle r, Color c, bool y_repeated) :
    IActor(game),
    dx(0.0F),
    r(r),
    color(c),
    y_repeated(y_repeated),
    behavior(std::make_unique<StaticBehaviorStrategy>()) {}

void Platform::draw() const {
    auto rect = r;
    auto &game = get_game();
    // Convert to screen coordinates
    rect.x -= game.get_rectangle().x;
    rect.y -= game.get_rectangle().y;

    DrawRectangleRec(rect, color);
}

void Platform::update(float delta) {
    auto r1 = r;
    behavior->update(*this, delta);
    dx = r.x - r1.x;
}

void Platform::process_input() {
    // Do nothing
}

void Platform::move(float x, float y) noexcept {
    r.x += x;
    r.y += y;
}
