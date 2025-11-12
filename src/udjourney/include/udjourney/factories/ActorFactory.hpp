// Copyright 2025 Quentin Cartier

#pragma once

#include <memory>

#include "udjourney/interfaces/IActor.hpp"

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
