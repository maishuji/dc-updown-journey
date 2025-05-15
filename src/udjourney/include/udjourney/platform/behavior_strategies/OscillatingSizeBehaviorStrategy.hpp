// Copyright 2025 Quentin Cartier
#ifndef PLATFORM_SHRINKINGBEHAVIORSTREATEGY_HPP
#define PLATFORM_SHRINKINGBEHAVIORSTREATEGY_HPP

#include <memory>

#include "udjourney/platform/behavior_strategies/PlatformBehaviorStrategy.hpp"

class Platform;

class OscillatingSizeBehaviorStrategy : public PlatformBehaviorStrategy {
 public:
    OscillatingSizeBehaviorStrategy(float iSpeedX, float iMinOffset,
                                    float iMaxOffset);
    ~OscillatingSizeBehaviorStrategy() override;
    void update(Platform& ioPlatform, float iDelta) override;

 private:
    struct PImpl;
    std::unique_ptr<PImpl> m_pimpl;
};

#endif  //  PLATFORM_SHRINKINGBEHAVIORSTREATEGY_HPP
