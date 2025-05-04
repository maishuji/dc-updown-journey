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

class StaticPlatformBehaviorStrategy : public PlatformBehaviorStrategy {
 public:
    void update(Platform& platform, float delta) override;
};

class HorizontalPlatformBehaviorStrategy : public PlatformBehaviorStrategy {
 public:
    HorizontalPlatformBehaviorStrategy();
    ~HorizontalPlatformBehaviorStrategy() override;
    void update(Platform& platform, float delta) override;

 private:
    struct PImpl;
    std::unique_ptr<PImpl> m_pimpl;
};
#endif  //  PLATFORMBEHAVIORSTRATEGY_HPP_
