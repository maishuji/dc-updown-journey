// Copyright 2025 Quentin Cartier
#ifndef PLATFORMBEHAVIORSTRATEGY_HPP_
#define PLATFORMBEHAVIORSTRATEGY_HPP_

#include <memory>

class Platform;
class PlatformBehaviorStrategy {
 public:
    virtual ~PlatformBehaviorStrategy() = default;
    virtual void update(Platform& platform, float delta) = 0;
};

class StaticBehaviorStrategy : public PlatformBehaviorStrategy {
 public:
    void update(Platform& platform, float delta) override;
};

#endif  //  PLATFORMBEHAVIORSTRATEGY_HPP_
