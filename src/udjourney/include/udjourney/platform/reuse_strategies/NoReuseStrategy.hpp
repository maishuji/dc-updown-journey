// Copyright 2025 Quentin Cartier
#pragma once
#include "udjourney/platform/reuse_strategies/PlatformReuseStrategy.hpp"

/**
 * @brief A reuse strategy that doesn't reuse platforms - marks them for removal
 * instead
 *
 * This strategy is used for level-based platforms that should not be reused
 * when they go out of scope, avoiding clutter on screen.
 */
class NoReuseStrategy : public PlatformReuseStrategy {
 public:
    void reuse(class Platform& platform) override;
};
