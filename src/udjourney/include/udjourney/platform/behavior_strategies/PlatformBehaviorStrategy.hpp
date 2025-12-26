// Copyright 2025 Quentin Cartier
#pragma once

#include <memory>
namespace udjourney {

class Platform;
class PlatformBehaviorStrategy {
 public:
    virtual ~PlatformBehaviorStrategy() = default;
    virtual void update(Platform& platform, float delta) = 0;
    virtual void reset() {}  // Optional reset method for behaviors that need it
};

class StaticBehaviorStrategy : public PlatformBehaviorStrategy {
 public:
    void update(Platform& platform, float delta) override;
};

}  // namespace udjourney
