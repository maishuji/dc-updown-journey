// Copyright 2025 Quentin Cartier
#pragma once
#include "udjourney/platform/reuse_strategies/PlatformReuseStrategy.hpp"
namespace udjourney {
class RandomizePositionStrategy : public PlatformReuseStrategy {
 public:
    void reuse(class Platform& platform) override;
};
}  // namespace udjourney
