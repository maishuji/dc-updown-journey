// Copyright 2025 Quentin Cartier

#include "udjourney/Bonus.hpp"

Bonus::Bonus(const IGame &game, Rectangle r) : IActor(game), r(r) {}

void Bonus::draw() const {
    auto rect = r;
    auto &game = get_game();
    // Convert to screen coordinates
    rect.x -= game.get_rectangle().x;
    rect.y -= game.get_rectangle().y;

    // Center of rotation: center of the rectangle
    Vector2 origin = {rect.width / 2.0f, rect.height / 2.0f};

    // Define rotation angle (you probably want it to increase over time)
    float rotation = GetTime() * 30.0f;  // for example, 90 degrees per second

    // Draw rotated rectangle
    DrawRectanglePro(rect, origin, rotation, YELLOW);
}

void Bonus::update(float delta) {
    // r.y -= 1;

    if (r.y < 0) {
        r.y = 480;
    }
}

void Bonus::process_input() {
    // Do nothing
}
