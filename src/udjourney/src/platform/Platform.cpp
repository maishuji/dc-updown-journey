// Copyright 2025 Quentin Cartier

#include "udjourney/platform/Platform.hpp"

#include <algorithm>

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
    Color color_red = RED;

    if (has_feature(PlatformFeature::SPIKES)) {
        DrawRectangleLinesEx(rect, 1.0F, color_red);

        // Draw spikes on top of the platform
        float spike_width = rect.width / 8.0f;
        for (int i = 0; i < 8; ++i) {
            float x = rect.x + i * spike_width;
            DrawTriangle(Vector2{x, rect.y},

                         Vector2{x + spike_width, rect.y},
                         Vector2{x + spike_width / 2, rect.y - 20},
                         RED);
        }
    }
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

void Platform::add_feature(PlatformFeature feature) {
    if (std::find(m_features.begin(), m_features.end(), feature) ==
        m_features.end())
        m_features.push_back(feature);
}

bool Platform::has_feature(PlatformFeature feature) const {
    return std::find(m_features.begin(), m_features.end(), feature) !=
           m_features.end();
}
