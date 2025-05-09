// Copyright 2025 Quentin Cartier
#ifndef PLATFORM_HORIZONTALBEHAVIORSTREATEGY_HPP
#define PLATFORM_HORIZONTALBEHAVIORSTREATEGY_HPP

#include <memory>

#include "udjourney/platform/behavior_strategies/PlatformBehaviorStrategy.hpp"

class Platform;
namespace{
inline const float kDefaultSpeed = 10.0F;
inline const float kDefaultOffset = 100.0F;
} // namespace
class HorizontalBehaviorStrategy : public PlatformBehaviorStrategy {
 public:
    HorizontalBehaviorStrategy(float speed_x = kDefaultSpeed,
                               float max_offset = 100.0F);
    ~HorizontalBehaviorStrategy() override;
    void update(Platform& platform, float delta) override;

 private:
    struct PImpl;
    std::unique_ptr<PImpl> m_pimpl;
};

#endif  //  PLATFORM_HORIZONTALBEHAVIORSTREATEGY_HPP
