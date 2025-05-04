// Copyright 2025 Quentin Cartier
#include "udjourney/platform/behavior_strategies/PlatformBehaviorStrategy.hpp"

#include <cmath>
#include <iostream>

#include "udjourney/platform/Platform.hpp"

struct HorizontalPlatformBehaviorStrategy::PImpl {
    float pivot_x = std::nanf("");  // Will be updated
    float max_offset = 100.0F;
    float factor = 1.0F;
    float speed_x = 10.0F;
};

HorizontalPlatformBehaviorStrategy::~HorizontalPlatformBehaviorStrategy() =
    default;

// Now define constructor
HorizontalPlatformBehaviorStrategy::HorizontalPlatformBehaviorStrategy() :
    m_pimpl(std::make_unique<PImpl>()) {}

void StaticPlatformBehaviorStrategy::update(Platform &platform, float delta) {
    // No movement
    const auto &gameRect = platform.get_game().get_rectangle();
    const auto &r = platform.get_rectangle();
    // Mark the platform as consummed if it goes out of the screen
    if (r.y + r.height < gameRect.y) {
        platform.set_state(ActorState::CONSUMED);
    }
}

void HorizontalPlatformBehaviorStrategy::update(Platform &platform,
                                                float delta) {
    // No movement
    const auto &gameRect = platform.get_game().get_rectangle();
    const auto &r = platform.get_rectangle();

    if (std::isnan(m_pimpl->pivot_x)) {
        // Init the pivot
        m_pimpl->pivot_x = r.x;
    }

    if (r.x < m_pimpl->pivot_x - m_pimpl->max_offset) {
        m_pimpl->factor = 1.0F;
    } else if (r.x > m_pimpl->pivot_x + m_pimpl->max_offset) {
        m_pimpl->factor = -1.0F;
    }

    platform.move(delta * m_pimpl->speed_x * m_pimpl->factor, 0.0F);

    if (r.y + r.height < gameRect.y) {
        platform.set_state(ActorState::CONSUMED);
    }
}
