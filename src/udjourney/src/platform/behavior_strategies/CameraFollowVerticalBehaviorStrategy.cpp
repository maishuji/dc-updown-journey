// Copyright 2025 Quentin Cartier
#include "udjourney/platform/behavior_strategies/CameraFollowVerticalBehaviorStrategy.hpp"

#include "udjourney/platform/Platform.hpp"

namespace udjourney {

struct CameraFollowVerticalBehaviorStrategy::PImpl {
    float offset_from_camera = 0.0f;
    float initial_x =
        0.0f;  // Store initial X position to maintain horizontal position
    float previous_camera_y = 0.0f;  // Track previous camera position
    bool initialized = false;
};

CameraFollowVerticalBehaviorStrategy::CameraFollowVerticalBehaviorStrategy(
    float offset_from_camera) :
    m_pimpl(std::make_unique<PImpl>()) {
    m_pimpl->offset_from_camera = offset_from_camera;
}

CameraFollowVerticalBehaviorStrategy::~CameraFollowVerticalBehaviorStrategy() =
    default;

void CameraFollowVerticalBehaviorStrategy::reset() {
    m_pimpl->initialized = false;
}

void CameraFollowVerticalBehaviorStrategy::update(Platform &platform,
                                                  float delta) {
    const auto &gameRect = platform.get_game().get_rectangle();
    auto rect = platform.get_rectangle();

    // Initialize on first update
    if (!m_pimpl->initialized) {
        m_pimpl->initial_x = rect.x;
        m_pimpl->previous_camera_y = gameRect.y;
        // Keep the initial position from the level data - don't override it
        m_pimpl->initialized = true;
        return;
    }

    // Calculate camera movement
    float camera_delta_y = gameRect.y - m_pimpl->previous_camera_y;
    m_pimpl->previous_camera_y = gameRect.y;

    // Move platform by the same amount the camera moved
    platform.move(0.0f, camera_delta_y);

    // Note: These platforms never get consumed since they move with the camera
    // They should only be created in scenes, not as random platforms
}

}  // namespace udjourney
