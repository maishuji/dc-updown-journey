// Copyright 2025 Quentin Cartier
#include "udjourney/platform/behavior_strategies/HorizontalBehaviorStrategy.hpp"

#include <cmath>
#include <iostream>

#include "udjourney/platform/Platform.hpp"
namespace udjourney {

struct HorizontalBehaviorStrategy::PImpl {
    float pivot_x = std::nanf("");  // Will be updated
    float max_offset = 100.0F;
    float factor = 1.0F;
    float speed_x = internal::kDefaultSpeed;
    float initial_offset = 0.0F;  // Starting offset from pivot
    float current_offset = 0.0F;  // Current offset from pivot
};

HorizontalBehaviorStrategy::~HorizontalBehaviorStrategy() = default;

// Now define constructor
HorizontalBehaviorStrategy::HorizontalBehaviorStrategy(float speed_x,
                                                       float max_offset,
                                                       float initial_offset) :
    m_pimpl(std::make_unique<PImpl>()) {
    m_pimpl->max_offset = max_offset;
    m_pimpl->speed_x = speed_x;
    m_pimpl->initial_offset = initial_offset;
    m_pimpl->current_offset = initial_offset;
}

void HorizontalBehaviorStrategy::reset() {
    // Reset to initial state
    m_pimpl->pivot_x = std::nanf("");
    m_pimpl->current_offset = m_pimpl->initial_offset;
    m_pimpl->factor = 1.0F;
}

void HorizontalBehaviorStrategy::update(Platform &platform, float delta) {
    // No movement
    const auto &gameRect = platform.get_game().get_rectangle();
    const auto &rect = platform.get_rectangle();

    if (std::isnan(m_pimpl->pivot_x)) {
        // Init the pivot and apply initial offset
        m_pimpl->pivot_x = rect.x - m_pimpl->initial_offset;
    }

    if (rect.x < m_pimpl->pivot_x - m_pimpl->max_offset) {
        m_pimpl->factor = 1.0F;
    } else if (rect.x > m_pimpl->pivot_x + m_pimpl->max_offset) {
        m_pimpl->factor = -1.0F;
    }

    platform.move(delta * m_pimpl->speed_x * m_pimpl->factor, 0.0F);
    m_pimpl->current_offset = rect.x - m_pimpl->pivot_x;

    if (rect.y + rect.height < gameRect.y) {
        platform.set_state(ActorState::CONSUMED);
    }
}
}  // namespace udjourney
