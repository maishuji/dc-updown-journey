// Copyright 2025 Quentin Cartier
#pragma once

#include <memory>

#include "udjourney/platform/behavior_strategies/PlatformBehaviorStrategy.hpp"

namespace udjourney {
class Platform;

class OscillatingSizeBehaviorStrategy : public PlatformBehaviorStrategy {
 public:
    OscillatingSizeBehaviorStrategy(float iSpeedX, float iMinOffset,
                                    float iMaxOffset);
    ~OscillatingSizeBehaviorStrategy() override;
    void update(Platform& ioPlatform, float iDelta) override;
    PlatformBehaviorType get_type() const override {
        return PlatformBehaviorType::OscillatingSize;
    }

 private:
    struct PImpl;
    std::unique_ptr<PImpl> m_pimpl;
};

}  // namespace udjourney
