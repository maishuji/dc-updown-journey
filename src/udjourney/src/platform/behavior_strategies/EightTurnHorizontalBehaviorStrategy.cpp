// Copyright 2025 Quentin Cartier
#include "udjourney/platform/behavior_strategies/EightTurnHorizontalBehaviorStrategy.hpp"

#include <cmath>
#include <iostream>

#include "udjourney/platform/Platform.hpp"

struct EightTurnHorizontalBehaviorStrategy::PImpl {
    float t = 0.0F;
    float speed = 0.01F;
    float max_offset = 300.0F;  // Maximum offset from the center
    float amplitude = 100.0F;
    float center_x = std::nanf("");
    float center_y = std::nanf("");
    float last_offset_x = 0.0F;  // Last x position
    float last_offset_y = 0.0F;  // Last x position
};

EightTurnHorizontalBehaviorStrategy::~EightTurnHorizontalBehaviorStrategy() =
    default;

EightTurnHorizontalBehaviorStrategy::EightTurnHorizontalBehaviorStrategy(
    float speed, float amplitude) :
    m_pimpl(std::make_unique<PImpl>()) {
    m_pimpl->speed = speed;
    m_pimpl->amplitude = amplitude;
}

void EightTurnHorizontalBehaviorStrategy::update(Platform &platform,
                                                 float delta) {
    const auto &gameRect = platform.get_game().get_rectangle();
    const auto &rect = platform.get_rectangle();
    std::cout << "EightTurnHorizontalBehaviorStrategy::update called" << delta
              << std::endl;
    if (std::isnan(m_pimpl->center_x)) {
        m_pimpl->center_x = rect.x;
        m_pimpl->center_y = rect.y;
    }
    m_pimpl->t += delta * m_pimpl->speed;
    m_pimpl->t += delta * m_pimpl->speed;
    // Lemniscate of Gerono: x = a * sin(t), y = a * sin(t) * cos(t)
    float y_offset = m_pimpl->amplitude * std::sin(m_pimpl->t);
    float x_offset =
        m_pimpl->amplitude * std::sin(m_pimpl->t) * std::cos(m_pimpl->t) / 2.0F;

    platform.move(-m_pimpl->last_offset_x + x_offset,
                  -m_pimpl->last_offset_y + y_offset);
    m_pimpl->last_offset_x = x_offset;
    m_pimpl->last_offset_y = y_offset;

    if (rect.y + rect.height < gameRect.y) {
        platform.set_state(ActorState::CONSUMED);
    }
}
