// Copyright 2025 Quentin Cartier

#include "udjourney/Bonus.hpp"

Bonus::Bonus(const IGame &iGame, Rectangle iRect) : IActor(iGame), m_rect(iRect) {}

void Bonus::draw() const {
    auto rect = m_rect;
    auto &game = get_game();
    // Convert to screen coordinates
    rect.x -= game.get_rectangle().x;
    rect.y -= game.get_rectangle().y;

    // Center of rotation: center of the rectangle
    Vector2 origin = {rect.width / 2.0F, rect.height / 2.0F};

    // Define rotation angle (you probably want it to increase over time)
    float rotation = static_cast<float>(GetTime() * 30.0);  // for example, 90 degrees per second

    // Draw rotated rectangle
    DrawRectanglePro(rect, origin, rotation, YELLOW);
}

void Bonus::update(float iDelta) {
    // r.y -= 1;

    if (m_rect.y < 0) {
        m_rect.y = 480;
    }
}

void Bonus::process_input() {
    // Do nothing
}
