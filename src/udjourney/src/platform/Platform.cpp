// Copyright 2025 Quentin Cartier

#include "udjourney/platform/Platform.hpp"

#include "udjourney/interfaces/IGame.hpp"

Platform::Platform(const IGame &iGame, Rectangle iRect, Color iColor,
                   bool iIsRepeatedY) :
    IActor(iGame),
    m_rect(iRect),
    m_color(iColor),
    m_repeated_y(iIsRepeatedY),
    m_behavior(std::make_unique<StaticBehaviorStrategy>()) {}

void Platform::draw() const {
    auto rect = m_rect;
    const auto &game = get_game();
    // Convert to screen coordinates
    rect.x -= game.get_rectangle().x;
    rect.y -= game.get_rectangle().y;

    DrawRectangleRec(rect, m_color);
}

void Platform::update(float iDelta) {
    auto original_rect = m_rect;
    m_behavior->update(*this, iDelta);
    m_delta_x = m_rect.x - original_rect.x;
}

void Platform::process_input() {
    // Do nothing
}

void Platform::move(float iValX, float iValY) noexcept {
    m_rect.x += iValX;
    m_rect.y += iValY;
}

void Platform::resize(float iNewWidth, float iNewHeight) noexcept {
    if (iNewHeight > 0 && iNewWidth > 0) {
        m_rect.width = iNewWidth;
        m_rect.height = iNewHeight;
    }
}
