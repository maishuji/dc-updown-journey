// Copyright 2025 Quentin Cartier
#pragma once

#include <memory>

#include "udjourney/platform/behavior_strategies/PlatformBehaviorStrategy.hpp"
namespace udjourney {
class Platform;

/**
 * @brief Platform behavior that moves at the same speed as the camera
 *
 * This strategy makes the platform move vertically at the same speed as the
 * camera. The platform maintains its initial Y position from the level data and
 * scrolls with the camera movement, staying at a consistent position relative
 * to the screen.
 *
 * Place these platforms at the desired Y position in your level JSON (e.g., y:
 * 0.5 for near the top of the screen). The platform will stay at that screen
 * position as the camera scrolls.
 */
class CameraFollowVerticalBehaviorStrategy : public PlatformBehaviorStrategy {
 public:
    /**
     * @brief Construct a new Camera Follow Vertical Behavior Strategy
     *
     * @param offset_from_camera Currently unused - position is determined by
     * level data
     */
    explicit CameraFollowVerticalBehaviorStrategy(
        float offset_from_camera = 0.0f);
    ~CameraFollowVerticalBehaviorStrategy() override;

    void update(Platform& platform, float delta) override;
    void reset() override;
    PlatformBehaviorType get_type() const override { return PlatformBehaviorType::CameraFollowVertical; }

 private:
    struct PImpl;
    std::unique_ptr<PImpl> m_pimpl;
};

}  // namespace udjourney
