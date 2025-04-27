// Copyright 2025 Quentin Cartier
#ifndef SRC_UDJOURNEY_INCLUDE_UDJOURNEY_PLATFORM_REUSE_STRATEGIES_RANDOMIZEPOSITIONSTRATEGY_HPP_
#define SRC_UDJOURNEY_INCLUDE_UDJOURNEY_PLATFORM_REUSE_STRATEGIES_RANDOMIZEPOSITIONSTRATEGY_HPP_

#include "udjourney/platform/reuse_strategies/PlatformReuseStrategy.hpp"

class RandomizePositionStrategy : public PlatformReuseStrategy {
 public:
    void reuse(class Platform& platform) override;
};

#endif  // SRC_UDJOURNEY_INCLUDE_UDJOURNEY_PLATFORM_REUSE_STRATEGIES_RANDOMIZEPOSITIONSTRATEGY_HPP_
