// Copyright 2025 Quentin Cartier
#include "udjourney/platform/behavior_strategies/OscillatingSizeBehaviorStrategy.hpp"

#include <cmath>
#include <iostream>

#include "udjourney/platform/Platform.hpp"

namespace {
const float kDefaultSpeed = 7.0F;
const float kDefaultOffset = 10.0F;
const float kMinSizeAcceptable = 5.0F;
const float kMaxSizeAcceptable = 500.0F;
}  // namespace

struct OscillatingSizeBehaviorStrategy::PImpl {
    float pivot_x = std::nanf("");         // Will be updated
    float original_width = std::nanf("");  // Will be updated
    float min_offset = kDefaultOffset;
    float max_offset = kDefaultOffset;
    float factor = 1.0F;
    float speed_x = kDefaultSpeed;
};

OscillatingSizeBehaviorStrategy::~OscillatingSizeBehaviorStrategy() = default;

// Now define constructor
OscillatingSizeBehaviorStrategy::OscillatingSizeBehaviorStrategy(
    float iSpeedX, float iMinOffset, float iMaxOffset) :
    m_pimpl(std::make_unique<PImpl>()) {
    if (iMinOffset > iMaxOffset) {
        std::cerr << "Wrong offsets provided in ShringBehaviorStrategy"
                  << std::endl;
        iMinOffset = iMaxOffset;
    }
    m_pimpl->min_offset = iMinOffset;
    m_pimpl->max_offset = iMaxOffset;

    m_pimpl->speed_x = std::abs(iSpeedX);
}

void OscillatingSizeBehaviorStrategy::update(Platform &ioPlatform,
                                             float iDelta) {
    // No movement
    const auto &gameRect = ioPlatform.get_game().get_rectangle();
    const auto &rect = ioPlatform.get_rectangle();

    if (std::isnan(m_pimpl->pivot_x)) {
        // Init the pivot (center x)
        m_pimpl->pivot_x = rect.x + rect.width / 2.0F;
        m_pimpl->original_width = rect.width;

        // Adjust min_offset if resulting size is below acceptable minimum
        float min_space = m_pimpl->original_width + 2.0F * m_pimpl->min_offset;
        if (min_space < kMinSizeAcceptable) {
            m_pimpl->min_offset =
                (kMinSizeAcceptable - m_pimpl->original_width) / 2.0F;
        }

        // Adjust max_offset if resulting size exceeds acceptable maximum
        float max_space = m_pimpl->original_width + 2.0F * m_pimpl->max_offset;
        if (max_space > kMaxSizeAcceptable) {
            m_pimpl->max_offset =
                (kMaxSizeAcceptable - m_pimpl->original_width) / 2.0F;
        }
        //std::cout << "min_offset " << m_pimpl->min_offset
        //          << " , max_offset: " << m_pimpl->max_offset << std::endl;
    }

    // Compute bounds
    const float max_width = m_pimpl->original_width + m_pimpl->max_offset;
    const float min_width = m_pimpl->original_width + m_pimpl->min_offset;

    //std::cout << "min_width " << min_width << " , max_width: " << max_width
    //          << " , speed_x: " << m_pimpl->factor << std::endl;
    //std::cout << "\t rect_width = " << rect.width << std::endl;
    // If bounds are hit, reverse direction
    if (rect.width >= max_width) {
        m_pimpl->factor = -1.0F;  // Start shrinking
    } else if (rect.width <= min_width) {
        m_pimpl->factor = 1.0F;  // Start growing
    }

    auto delta_x = iDelta * m_pimpl->speed_x * m_pimpl->factor;

    ioPlatform.move(-delta_x, 0.0F);
    ioPlatform.resize(rect.width + 2.0F * delta_x, rect.height);

    if (rect.y + rect.height < gameRect.y) {
        ioPlatform.set_state(ActorState::CONSUMED);
    }
}
