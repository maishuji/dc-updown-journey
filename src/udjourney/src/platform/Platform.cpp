// Copyright 2025 Quentin Cartier

#include "udjourney/platform/Platform.hpp"

#include <algorithm>
#include <any>
#include <map>
#include <memory>
#include <utility>

#include "udjourney/interfaces/IGame.hpp"

Platform::Platform(const IGame &iGame, Rectangle iRect, Color iColor,
                   bool iIsRepeatedY) :
    IActor(iGame),
    m_rect(iRect),
    m_color(iColor),
    m_repeated_y(iIsRepeatedY),
    m_behavior(std::make_unique<StaticBehaviorStrategy>()) {}

Rectangle Platform::get_drawing_rect() const {
    auto rect = m_rect;
    const auto &game = get_game();
    // Convert to screen coordinates
    rect.x -= game.get_rectangle().x;
    rect.y -= game.get_rectangle().y;
    return rect;
}

void Platform::draw() const {
    auto rect = m_rect;
    const auto &game = get_game();
    // Convert to screen coordinates
    rect.x -= game.get_rectangle().x;
    rect.y -= game.get_rectangle().y;

    DrawRectangleRec(rect, m_color);
    Color color_red = RED;

    for (const auto &feature : m_features) {
        feature->draw(*this);
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

void Platform::add_feature(std::unique_ptr<PlatformFeatureBase> feature) {
    int_fast8_t new_type = feature->get_type();
    auto it =
        std::find_if(m_features.begin(),
                     m_features.end(),
                     [new_type](const std::unique_ptr<PlatformFeatureBase> &f) {
                         return f->get_type() == new_type;
                     });
    if (it == m_features.end()) {
        m_features.push_back(std::move(feature));
    }
}
