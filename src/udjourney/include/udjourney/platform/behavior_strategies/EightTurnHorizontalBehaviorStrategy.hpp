// Copyright 2025 Quentin Cartier
#pragma once
#include <memory>
#include "udjourney/platform/behavior_strategies/PlatformBehaviorStrategy.hpp"
namespace udjourney {
class Platform;

class EightTurnHorizontalBehaviorStrategy : public PlatformBehaviorStrategy {
 public:
    EightTurnHorizontalBehaviorStrategy(float speed = 1.0F,
                                        float amplitude = 100.0F);
    ~EightTurnHorizontalBehaviorStrategy() override;
    void update(Platform &platform, float delta) override;
    PlatformBehaviorType get_type() const override {
        return PlatformBehaviorType::EightTurnHorizontal;
    }

 private:
    struct PImpl;
    std::unique_ptr<PImpl> m_pimpl;
};
}  // namespace udjourney
