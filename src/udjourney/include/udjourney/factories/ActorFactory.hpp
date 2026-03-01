// Copyright 2025 Quentin Cartier

#pragma once

#include <raylib/raylib.h>
#include <memory>

#include "udjourney/interfaces/IActor.hpp"
#include "udjourney/scene/LevelPhysicsConfig.hpp"

// Forward declaration
namespace udjourney {
namespace core {
namespace events {
class EventDispatcher;
}
}  // namespace core

namespace scene {
struct MonsterSpawnData;
}  // namespace scene

}  // namespace udjourney

namespace udjourney {
class ActorFactory {
 public:
    ActorFactory() = default;
    virtual ~ActorFactory() = default;

    virtual std::unique_ptr<IActor> create_actor() = 0;
};

class MonsterFactory : public ActorFactory {
 public:
    MonsterFactory(const IGame &game,
                   udjourney::core::events::EventDispatcher &event_dispatcher);
    std::unique_ptr<IActor> create_actor() override;
    std::unique_ptr<IActor> create_actor(Rectangle rect);
    std::unique_ptr<IActor> create_actor_from_monster_data(
        const scene::MonsterSpawnData &monster_data);

    void set_physics_config(const scene::LevelPhysicsConfig &config) {
        physics_config_ = config;
    }

 private:
    const IGame &m_game;
    udjourney::core::events::EventDispatcher &m_event_dispatcher;
    scene::LevelPhysicsConfig physics_config_;
};

class PlayerFactory : public ActorFactory {
 public:
    PlayerFactory(const IGame &game,
                  udjourney::core::events::EventDispatcher &event_dispatcher);
    std::unique_ptr<IActor> create_actor() override;
    std::unique_ptr<IActor> create_actor(Rectangle rect);

    void set_physics_config(const scene::LevelPhysicsConfig &config) {
        physics_config_ = config;
    }

 private:
    const IGame &m_game;
    udjourney::core::events::EventDispatcher &m_event_dispatcher;
    scene::LevelPhysicsConfig physics_config_;
};
}  // namespace udjourney
