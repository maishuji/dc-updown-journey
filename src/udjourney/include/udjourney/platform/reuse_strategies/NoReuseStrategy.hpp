// Copyright 2025 Quentin Cartier
#ifndef SRC_UDJOURNEY_INCLUDE_UDJOURNEY_PLATFORM_REUSE_STRATEGIES_NOREUSESTRATEGY_HPP_
#define SRC_UDJOURNEY_INCLUDE_UDJOURNEY_PLATFORM_REUSE_STRATEGIES_NOREUSESTRATEGY_HPP_

#include "udjourney/platform/reuse_strategies/PlatformReuseStrategy.hpp"

/**
 * @brief A reuse strategy that doesn't reuse platforms - marks them for removal instead
 * 
 * This strategy is used for level-based platforms that should not be reused
 * when they go out of scope, avoiding clutter on screen.
 */
class NoReuseStrategy : public PlatformReuseStrategy {
 public:
    void reuse(class Platform& platform) override;
};

#endif  // SRC_UDJOURNEY_INCLUDE_UDJOURNEY_PLATFORM_REUSE_STRATEGIES_NOREUSESTRATEGY_HPP_