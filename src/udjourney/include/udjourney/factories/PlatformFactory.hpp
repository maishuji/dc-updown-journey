// Copyright 2025 Quentin Cartier
#pragma once

#include <raylib/raylib.h>

#include <memory>

#include "udjourney/interfaces/IGame.hpp"
#include "udjourney/platform/Platform.hpp"
#include "udjourney/scene/Scene.hpp"

namespace udjourney {

class PlatformFactory {
 public:
    static std::unique_ptr<Platform> create(
        const IGame& game, const Rectangle& world_rect,
        const udjourney::scene::PlatformData& data);
};

}  // namespace udjourney
