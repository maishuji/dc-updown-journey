// Copyright 2025 Quentin Cartier
#pragma once

#include <utility>
#include <string>

#include "udjourney/interfaces/IActorState.hpp"
#include "udjourney/MonsterPreset.hpp"

class Monster;

// Base class for all monster states with a name
class MonsterStateBase : public IActorState {
 public:
    explicit MonsterStateBase(std::string name) : name_(std::move(name)) {}
    virtual ~MonsterStateBase() = default;

    [[nodiscard]] std::string get_name() const { return name_; }
    virtual void on_enter() {}
    virtual void on_exit() {}
    virtual void on_resume() {}

 protected:
    std::string name_;
    float state_timer_ = 0.0f;

    // Data-driven transition checking
    void check_transitions(IActor& actor, float delta);
    bool should_transition(IActor& actor,
                           const udjourney::StateTransition& transition);
};

// Idle State - Monster waits and occasionally looks around
class MonsterIdleState : public MonsterStateBase {
 public:
    MonsterIdleState() : MonsterStateBase("idle") {}
    ~MonsterIdleState() override = default;

    void enter(IActor& actor) override;
    void handleInput(IActor& actor) override;
    void update(IActor& actor, float delta) override;
};

// Patrol State - Monster walks back and forth
class MonsterPatrolState : public MonsterStateBase {
 public:
    MonsterPatrolState() : MonsterStateBase("patrol") {}
    ~MonsterPatrolState() override = default;

    void enter(IActor& actor) override;
    void handleInput(IActor& actor) override;
    void update(IActor& actor, float delta) override;
};

// Chase State - Monster pursues the player
class MonsterChaseState : public MonsterStateBase {
 public:
    MonsterChaseState() : MonsterStateBase("chase") {}
    ~MonsterChaseState() override = default;

    void enter(IActor& actor) override;
    void handleInput(IActor& actor) override;
    void update(IActor& actor, float delta) override;
};

// Attack State - Monster attacks the player
class MonsterAttackState : public MonsterStateBase {
 public:
    MonsterAttackState() : MonsterStateBase("attack") {}
    ~MonsterAttackState() override = default;

    void enter(IActor& actor) override;
    void handleInput(IActor& actor) override;
    void update(IActor& actor, float delta) override;
};

// Hurt State - Monster is damaged
class MonsterHurtState : public MonsterStateBase {
 public:
    MonsterHurtState() : MonsterStateBase("hurt") {}
    ~MonsterHurtState() override = default;

    void enter(IActor& actor) override;
    void handleInput(IActor& actor) override;
    void update(IActor& actor, float delta) override;
};

// Death State - Monster dies
class MonsterDeathState : public MonsterStateBase {
 public:
    MonsterDeathState() : MonsterStateBase("death") {}
    ~MonsterDeathState() override = default;

    void enter(IActor& actor) override;
    void handleInput(IActor& actor) override;
    void update(IActor& actor, float delta) override;
};
