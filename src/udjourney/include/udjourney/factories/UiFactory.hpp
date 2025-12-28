// Copyright 2025 Quentin Cartier

#pragma once

#include <memory>
#include <vector>

#include <raylib/raylib.h>

#include "udjourney/core/events/EventDispatcher.hpp"
#include "udjourney/hud/scene/IHUD.hpp"
#include "udjourney/interfaces/IActor.hpp"
#include "udjourney/scene/Scene.hpp"

namespace udjourney {

class Game;

class UiFactory {
 public:
    struct Result {
        std::vector<std::unique_ptr<udjourney::hud::scene::IHUD>> scene_huds;
        std::vector<std::unique_ptr<udjourney::IActor>> widget_actors;
    };

    static Result create(
        const udjourney::scene::Scene& scene, udjourney::Game& game,
        udjourney::core::events::EventDispatcher& event_dispatcher);
};

}  // namespace udjourney
