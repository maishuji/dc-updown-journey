// Copyright 2025 Quentin Cartier
#pragma once

#include <memory>

#include "udjourney/platform/behavior_strategies/PlatformBehaviorStrategy.hpp"
namespace udjourney {
class Platform;
namespace internal {
inline const float kDefaultSpeed = 10.0F;
inline const float kDefaultOffset = 100.0F;
}  // namespace internal
class HorizontalBehaviorStrategy : public PlatformBehaviorStrategy {
 public:
    HorizontalBehaviorStrategy(float speed_x = internal::kDefaultSpeed,
                               float max_offset = internal::kDefaultOffset,
                               float initial_offset = 0.0f);
    ~HorizontalBehaviorStrategy() override;
    void update(Platform& platform, float delta) override;
    void reset() override;
    PlatformBehaviorType get_type() const override {
        return PlatformBehaviorType::Horizontal;
    }

 private:
    struct PImpl;
    std::unique_ptr<PImpl> m_pimpl;
};

}  // namespace udjourney
