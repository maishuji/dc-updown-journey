// Copyright 2025 Quentin Cartier
#pragma once

#include <memory>
namespace udjourney {

class Platform;

enum class PlatformBehaviorType {
    Static,
    Horizontal,
    OscillatingSize,
    EightTurnHorizontal,
    CameraFollowVertical
};

class PlatformBehaviorStrategy {
 public:
    virtual ~PlatformBehaviorStrategy() = default;
    virtual void update(Platform& platform, float delta) = 0;
    virtual void reset() {}  // Optional reset method for behaviors that need it
    virtual PlatformBehaviorType get_type() const = 0;
};

class StaticBehaviorStrategy : public PlatformBehaviorStrategy {
 public:
    void update(Platform& platform, float delta) override;
    PlatformBehaviorType get_type() const override { return PlatformBehaviorType::Static; }
};

}  // namespace udjourney
