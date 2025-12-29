// Copyright 2025 Quentin Cartier

#pragma once

#include <raylib/raylib.h>
#include <memory>

#include "udjourney/interfaces/IActor.hpp"

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

 private:
    const IGame &m_game;
    udjourney::core::events::EventDispatcher &m_event_dispatcher;
};

class PlayerFactory : public ActorFactory {
 public:
    PlayerFactory(const IGame &game,
                  udjourney::core::events::EventDispatcher &event_dispatcher);
    std::unique_ptr<IActor> create_actor() override;
    std::unique_ptr<IActor> create_actor(Rectangle rect);

 private:
    const IGame &m_game;
    udjourney::core::events::EventDispatcher &m_event_dispatcher;
};
}  // namespace udjourney
