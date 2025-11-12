// Copyright 2025 Quentin Cartier

#pragma once

#include <memory>
#include <raylib/raylib.h>

#include "udjourney/interfaces/IActor.hpp"

// Forward declaration
namespace udjourney {
namespace core {
namespace events {
class EventDispatcher;
}
}  // namespace core
}  // namespace udjourney

class ActorFactory {
 public:
    ActorFactory() = default;
    virtual ~ActorFactory() = default;

    virtual std::unique_ptr<IActor> create_actor() = 0;
};

class MonsterFactory : public ActorFactory {
 public:
    MonsterFactory(const IGame &game, Rectangle rect);
    std::unique_ptr<IActor> create_actor() override;

 private:
    const IGame &m_game;
    Rectangle m_rect;
};

class PlayerFactory : public ActorFactory {
 public:
    PlayerFactory(const IGame &game, Rectangle rect,
                  udjourney::core::events::EventDispatcher &event_dispatcher);
    std::unique_ptr<IActor> create_actor() override;

 private:
    const IGame &m_game;
    Rectangle m_rect;
    udjourney::core::events::EventDispatcher &m_event_dispatcher;
};
