// Copyright 2025 Quentin Cartier
#include "udjourney/platform/behavior_strategies/ShrinkingBehaviorStrategy.hpp"

#include <cmath>
#include <iostream>

#include "udjourney/platform/Platform.hpp"

namespace {
const float kDefaultSpeed = 7.0F;
}

struct ShrinkingBehaviorStrategy::PImpl {
    float pivot_x = std::nanf("");         // Will be updated
    float original_width = std::nanf("");  // Will be updated
    float max_offset = 100.0F;
    float factor = 1.0F;
    float speed_x = kDefaultSpeed;
};

ShrinkingBehaviorStrategy::~ShrinkingBehaviorStrategy() = default;

// Now define constructor
ShrinkingBehaviorStrategy::ShrinkingBehaviorStrategy(float iSpeedX,
                                                     float iMaxOffset) :
    m_pimpl(std::make_unique<PImpl>()) {
    m_pimpl->max_offset = iMaxOffset;
    m_pimpl->speed_x = iSpeedX;
}

void ShrinkingBehaviorStrategy::update(Platform &ioPlatform, float iDelta) {
    // No movement
    const auto &gameRect = ioPlatform.get_game().get_rectangle();
    const auto &rect = ioPlatform.get_rectangle();

    if (std::isnan(m_pimpl->pivot_x)) {
        // Init the pivot (center x)
        m_pimpl->pivot_x = rect.x + rect.width / 2.0F;
        m_pimpl->original_width = rect.width;

        if (m_pimpl->original_width - m_pimpl->max_offset < 0) {
            // Reajust offset
            m_pimpl->max_offset = m_pimpl->original_width / 2.0F;
        }
    }

    if (rect.width > m_pimpl->original_width + m_pimpl->max_offset) {
        m_pimpl->factor = 1.0F;
    } else if (rect.width < m_pimpl->original_width - m_pimpl->max_offset) {
        m_pimpl->factor = -1.0F;
    }

    auto delta_x = iDelta * m_pimpl->speed_x * m_pimpl->factor;

    ioPlatform.move(delta_x, 0.0F);
    ioPlatform.resize(rect.width - 2.0F * delta_x, rect.height);

    if (rect.y + rect.height < gameRect.y) {
        ioPlatform.set_state(ActorState::CONSUMED);
    }
}
