// Copyright 2025 Quentin Cartier
#include "udjourney/platform/reuse_strategies/NoReuseStrategy.hpp"

#include "udjourney/platform/Platform.hpp"

void NoReuseStrategy::reuse(Platform& platform) {
    // Level-based platforms should not be reused - mark for removal
    // This prevents scene-based platforms from cluttering the screen
    platform.set_state(ActorState::CONSUMED);
}
