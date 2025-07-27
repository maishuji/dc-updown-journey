// Copyright 2025 Quentin Cartier
#pragma once
#include <memory>
#include "udjourney/platform/behavior_strategies/PlatformBehaviorStrategy.hpp"
class Platform;

class EightTurnHorizontalBehaviorStrategy : public PlatformBehaviorStrategy {
 public:
    EightTurnHorizontalBehaviorStrategy(float speed = 1.0F,
                                        float amplitude = 100.0F);
    ~EightTurnHorizontalBehaviorStrategy() override;
    void update(Platform &platform, float delta) override;

 private:
    struct PImpl;
    std::unique_ptr<PImpl> m_pimpl;
};
