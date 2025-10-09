// Copyright 2025 Quentin Cartier
#pragma once

#include <cstdint>

// Forward declarations
class Platform;
class IActor;

struct PlatformFeatureBase {
    virtual int_fast8_t get_type() const { return 0; }
    virtual ~PlatformFeatureBase() = default;
    virtual void draw(const Platform&) const {}
    virtual void handle_collision(Platform&, class IActor&) {}
};
