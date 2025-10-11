// Copyright 2025 Quentin Cartier
#pragma once

#include <cstdint>

#include "udjourney/platform/features/PlatformFeatureBase.hpp"

// Forward declaration
class Platform;

class CheckpointFeature : public PlatformFeatureBase {
 public:
    CheckpointFeature() = default;
    ~CheckpointFeature() override = default;

    int_fast8_t get_type() const override { return 2; }  // Checkpoint type

    void draw(const Platform& platform) const override;
    void handle_collision(Platform& platform, class IActor& actor) override;
};
