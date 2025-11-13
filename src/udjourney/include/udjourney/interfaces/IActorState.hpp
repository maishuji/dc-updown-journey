// Copyright 2025 Quentin Cartier
#pragma once

class IActor;

class IActorState {
 public:
    virtual ~IActorState() = default;

    virtual void enter(IActor& actor) = 0;

    virtual void handleInput(IActor& actor) = 0;
    virtual void update(IActor& actor, float delta) = 0;
};
