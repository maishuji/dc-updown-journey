// Copyright 2025 Quentin Cartier
#include "udjourney/states/MonsterStates.hpp"

#include <cmath>
#include <iostream>

#include "udj-core/Logger.hpp"
#include "udjourney/Monster.hpp"
#include "udjourney/Player.hpp"

namespace udjourney {

// Helper function to cast IActor to Monster
static Monster& to_monster(IActor& actor) {
    return static_cast<Monster&>(actor);
}

// ===== Base State with Data-Driven Transition Logic =====

void MonsterStateBase::check_transitions(IActor& actor, float delta) {
    auto& monster = to_monster(actor);
    const auto& preset = monster.get_preset();

    // Skip transitions if no preset is loaded
    if (!preset) {
        return;  // No preset-based transitions, monster uses basic behavior
    }

    // Check all possible transitions from current state
    for (const auto& transition : preset->state_config.transitions) {
        if (transition.from_state != this->name_) {
            continue;  // Not applicable to current state
        }

        if (should_transition(actor, transition)) {
            monster.change_state(transition.to_state);
            return;
        }
    }
}

bool MonsterStateBase::should_transition(
    IActor& actor, const udjourney::StateTransition& transition) {
    auto& monster = to_monster(actor);

    if (transition.condition == "player_in_range") {
        auto* player_ptr = monster.find_player();
        if (!player_ptr) return false;

        float dx = monster.get_rectangle().x - player_ptr->get_rectangle().x;
        float dy = monster.get_rectangle().y - player_ptr->get_rectangle().y;
        float distance = std::sqrt(dx * dx + dy * dy);
        return distance <= transition.condition_value;

    } else if (transition.condition == "player_out_of_range") {
        auto* player_ptr = monster.find_player();
        if (!player_ptr) return true;

        float dx = monster.get_rectangle().x - player_ptr->get_rectangle().x;
        float dy = monster.get_rectangle().y - player_ptr->get_rectangle().y;
        float distance = std::sqrt(dx * dx + dy * dy);
        return distance > transition.condition_value;

    } else if (transition.condition == "timer_expired") {
        return state_timer_ >= transition.condition_value;

    } else if (transition.condition == "animation_finished") {
        return monster.is_animation_finished();

    } else if (transition.condition == "grounded") {
        return monster.is_grounded();

    } else if (transition.condition == "health_below") {
        return monster.get_current_health() < transition.condition_value;
    }

    return false;
}

// ===== MonsterIdleState =====

void MonsterIdleState::enter(IActor& actor) {
    auto& monster = to_monster(actor);
    monster.set_velocity_x(0.0f);
    state_timer_ = 0.0f;
    udj::core::Logger::info("Monster entered IDLE state");
}

void MonsterIdleState::handleInput(IActor& actor) {
    // Idle state doesn't handle input
}

void MonsterIdleState::update(IActor& actor, float delta) {
    auto& monster = to_monster(actor);
    state_timer_ += delta;

    // Check data-driven transitions
    check_transitions(actor, delta);
}

// ===== MonsterPatrolState =====

void MonsterPatrolState::enter(IActor& actor) {
    auto& monster = to_monster(actor);
    // Start moving in the current facing direction
    monster.set_velocity_x(monster.is_facing_right()
                               ? monster.get_patrol_speed()
                               : -monster.get_patrol_speed());
    state_timer_ = 0.0f;
    udj::core::Logger::info("Monster entered PATROL state");
}

void MonsterPatrolState::handleInput(IActor& actor) {
    // Patrol state doesn't handle input
}

void MonsterPatrolState::update(IActor& actor, float delta) {
    auto& monster = to_monster(actor);
    state_timer_ += delta;

    // Check patrol boundaries
    auto rect = monster.get_rectangle();
    if (monster.is_facing_right() &&
        rect.x >= monster.get_patrol_right_bound()) {
        monster.reverse_direction();
    } else if (!monster.is_facing_right() &&
               rect.x <= monster.get_patrol_left_bound()) {
        monster.reverse_direction();
    }

    // Check data-driven transitions
    check_transitions(actor, delta);
}

// ===== MonsterChaseState =====

void MonsterChaseState::enter(IActor& actor) {
    auto& monster = to_monster(actor);
    state_timer_ = 0.0f;
    udj::core::Logger::info("Monster entered CHASE state");
}

void MonsterChaseState::handleInput(IActor& actor) {
    // Chase state doesn't handle input
}

void MonsterChaseState::update(IActor& actor, float delta) {
    auto& monster = to_monster(actor);
    state_timer_ += delta;

    auto* player_ptr = monster.find_player();
    if (player_ptr) {
        // Chase the player
        float player_x = player_ptr->get_rectangle().x;
        float monster_x = monster.get_rectangle().x;

        if (player_x > monster_x) {
            monster.set_velocity_x(monster.get_chase_speed());
            monster.set_facing_right(true);
        } else {
            monster.set_velocity_x(-monster.get_chase_speed());
            monster.set_facing_right(false);
        }
    }

    // Check data-driven transitions
    check_transitions(actor, delta);
}

// ===== MonsterAttackState =====

void MonsterAttackState::enter(IActor& actor) {
    auto& monster = to_monster(actor);
    monster.set_velocity_x(0.0f);
    state_timer_ = 0.0f;
    udj::core::Logger::info("Monster entered ATTACK state");
}

void MonsterAttackState::handleInput(IActor& actor) {
    // Attack state doesn't handle input
}

void MonsterAttackState::update(IActor& actor, float delta) {
    auto& monster = to_monster(actor);
    state_timer_ += delta;

    // Move towards player during attack
    auto* player_ptr = monster.find_player();
    if (player_ptr) {
        float player_x = player_ptr->get_rectangle().x;
        float monster_x = monster.get_rectangle().x;

        if (player_x > monster_x) {
            monster.set_velocity_x(monster.get_chase_speed());
            monster.set_facing_right(true);
        } else {
            monster.set_velocity_x(-monster.get_chase_speed());
            monster.set_facing_right(false);
        }
    } else {
        // No player found, stay stationary
        monster.set_velocity_x(0.0f);
    }

    // Check if attack animation is finished
    if (monster.is_animation_finished()) {
        udj::core::Logger::info(
            "Attack animation finished, returning to chase");
        // Return to chase state after attack completes
        monster.change_state("chase");
        return;
    }

    // Check data-driven transitions
    check_transitions(actor, delta);
}

// ===== MonsterHurtState =====

void MonsterHurtState::enter(IActor& actor) {
    auto& monster = to_monster(actor);
    monster.set_velocity_x(0.0f);
    state_timer_ = 0.0f;
    udj::core::Logger::info("Monster entered HURT state");
}

void MonsterHurtState::handleInput(IActor& actor) {
    // Hurt state doesn't handle input
}

void MonsterHurtState::update(IActor& actor, float delta) {
    auto& monster = to_monster(actor);
    state_timer_ += delta;

    // Monster stays stationary during hurt animation
    monster.set_velocity_x(0.0f);

    // Check if hurt animation is finished
    if (monster.is_animation_finished()) {
        udj::core::Logger::info("Hurt animation finished, returning to patrol");
        // Return to patrol state after hurt animation completes
        monster.change_state("patrol");
        return;
    }

    // Check data-driven transitions
    check_transitions(actor, delta);
}

// ===== MonsterDeathState =====

void MonsterDeathState::enter(IActor& actor) {
    auto& monster = to_monster(actor);
    monster.set_velocity_x(0.0f);
    state_timer_ = 0.0f;
    udj::core::Logger::info("Monster entered DEATH state");
}

void MonsterDeathState::handleInput(IActor& actor) {
    // Death state doesn't handle input
}

void MonsterDeathState::update(IActor& actor, float delta) {
    auto& monster = to_monster(actor);
    state_timer_ += delta;

    // Monster stays stationary during death animation
    monster.set_velocity_x(0.0f);

    // Check if death animation is finished
    if (monster.is_animation_finished()) {
        udj::core::Logger::info(
            "Death animation finished, marking monster as CONSUMED");

        // Award points for killing the monster
        monster.award_kill_points();

        actor.set_state(ActorState::CONSUMED);
        return;
    }

    // Check data-driven transitions (though death is usually final)
    check_transitions(actor, delta);
}
}  // namespace udjourney
