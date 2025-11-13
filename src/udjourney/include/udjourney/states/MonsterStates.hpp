// Copyright 2025 Quentin Cartier
#pragma once

#include <utility>
#include <string>

#include "udjourney/interfaces/IActorState.hpp"

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
};

// Idle State - Monster waits and occasionally looks around
class MonsterIdleState : public MonsterStateBase {
 public:
    MonsterIdleState() : MonsterStateBase("IDLE") {}
    ~MonsterIdleState() override = default;

    void enter(IActor& actor) override;
    void handleInput(IActor& actor) override;
    void update(IActor& actor, float delta) override;

 private:
    float idle_timer_ = 0.0f;
};

// Patrol State - Monster walks back and forth
class MonsterPatrolState : public MonsterStateBase {
 public:
    MonsterPatrolState() : MonsterStateBase("PATROL") {}
    ~MonsterPatrolState() override = default;

    void enter(IActor& actor) override;
    void handleInput(IActor& actor) override;
    void update(IActor& actor, float delta) override;
};

// Chase State - Monster pursues the player
class MonsterChaseState : public MonsterStateBase {
 public:
    MonsterChaseState() : MonsterStateBase("CHASE") {}
    ~MonsterChaseState() override = default;

    void enter(IActor& actor) override;
    void handleInput(IActor& actor) override;
    void update(IActor& actor, float delta) override;
};

// Attack State - Monster attacks the player
class MonsterAttackState : public MonsterStateBase {
 public:
    MonsterAttackState() : MonsterStateBase("ATTACK") {}
    ~MonsterAttackState() override = default;

    void enter(IActor& actor) override;
    void handleInput(IActor& actor) override;
    void update(IActor& actor, float delta) override;

 private:
    float attack_timer_ = 0.0f;
};

// Hurt State - Monster is damaged
class MonsterHurtState : public MonsterStateBase {
 public:
    MonsterHurtState() : MonsterStateBase("HURT") {}
    ~MonsterHurtState() override = default;

    void enter(IActor& actor) override;
    void handleInput(IActor& actor) override;
    void update(IActor& actor, float delta) override;

 private:
    float hurt_timer_ = 0.0f;
};

// Death State - Monster dies
class MonsterDeathState : public MonsterStateBase {
 public:
    MonsterDeathState() : MonsterStateBase("DEATH") {}
    ~MonsterDeathState() override = default;

    void enter(IActor& actor) override;
    void handleInput(IActor& actor) override;
    void update(IActor& actor, float delta) override;

 private:
    float death_timer_ = 0.0f;
};
