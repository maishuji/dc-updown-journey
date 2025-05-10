// Copyright 2025 Quentin Cartier
#include "udjourney/platform/behavior_strategies/PlatformBehaviorStrategy.hpp"

#include <cmath>
#include <iostream>

#include "udjourney/platform/Platform.hpp"

void StaticBehaviorStrategy::update(Platform &platform, float delta) {
    // No movement
    const auto &gameRect = platform.get_game().get_rectangle();
    const auto &rect = platform.get_rectangle();
    // Mark the platform as consummed if it goes out of the screen
    if (rect.y + rect.height < gameRect.y) {
        platform.set_state(ActorState::CONSUMED);
    }
}
