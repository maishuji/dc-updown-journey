// Copyright 2025 Quentin Cartier
#ifndef PLATFORM_SHRINKINGBEHAVIORSTREATEGY_HPP
#define PLATFORM_SHRINKINGBEHAVIORSTREATEGY_HPP

#include <memory>

#include "udjourney/platform/behavior_strategies/PlatformBehaviorStrategy.hpp"

class Platform;

class ShrinkingBehaviorStrategy : public PlatformBehaviorStrategy {
 public:
    ShrinkingBehaviorStrategy(float iSpeedX, float iMaxOffset);
    ~ShrinkingBehaviorStrategy() override;
    void update(Platform& platform, float delta) override;

 private:
    struct PImpl;
    std::unique_ptr<PImpl> m_pimpl;
};

#endif  //  PLATFORM_SHRINKINGBEHAVIORSTREATEGY_HPP
