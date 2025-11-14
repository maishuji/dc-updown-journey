// Copyright 2025 Quentin Cartier
#include "udjourney/states/MonsterStates.hpp"

#include <cmath>
#include <iostream>

#include "udjourney/Monster.hpp"
#include "udjourney/Player.hpp"

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
        if (transition.from_state != get_name()) {
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

        float distance =
            std::abs(monster.get_rectangle().x - player_ptr->get_rectangle().x);
        return distance < transition.condition_value;

    } else if (transition.condition == "player_out_of_range") {
        auto* player_ptr = monster.find_player();
        if (!player_ptr) return true;

        float distance =
            std::abs(monster.get_rectangle().x - player_ptr->get_rectangle().x);
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
    std::cout << "Monster entered IDLE state\n";
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
    std::cout << "Monster entered PATROL state\n";
}

void MonsterPatrolState::handleInput(IActor& actor) {
    // Patrol state doesn't handle input
}

void MonsterPatrolState::update(IActor& actor, float delta) {
    auto& monster = to_monster(actor);
    state_timer_ += delta;

    std::cout << "Monster patrolling at velocity X: "
              << monster.get_patrol_speed() << "\n";

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
    std::cout << "Monster entered CHASE state\n";
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
    std::cout << "Monster entered ATTACK state\n";
}

void MonsterAttackState::handleInput(IActor& actor) {
    // Attack state doesn't handle input
}

void MonsterAttackState::update(IActor& actor, float delta) {
    auto& monster = to_monster(actor);
    state_timer_ += delta;

    // Monster stays stationary during attack
    monster.set_velocity_x(0.0f);

    // Check data-driven transitions
    check_transitions(actor, delta);
}

// ===== MonsterHurtState =====

void MonsterHurtState::enter(IActor& actor) {
    auto& monster = to_monster(actor);
    monster.set_velocity_x(0.0f);
    state_timer_ = 0.0f;
    std::cout << "Monster entered HURT state\n";
}

void MonsterHurtState::handleInput(IActor& actor) {
    // Hurt state doesn't handle input
}

void MonsterHurtState::update(IActor& actor, float delta) {
    auto& monster = to_monster(actor);
    state_timer_ += delta;

    // Monster stays stationary during hurt animation
    monster.set_velocity_x(0.0f);

    // Check data-driven transitions
    check_transitions(actor, delta);
}

// ===== MonsterDeathState =====

void MonsterDeathState::enter(IActor& actor) {
    auto& monster = to_monster(actor);
    monster.set_velocity_x(0.0f);
    state_timer_ = 0.0f;
    std::cout << "Monster entered DEATH state\n";
}

void MonsterDeathState::handleInput(IActor& actor) {
    // Death state doesn't handle input
}

void MonsterDeathState::update(IActor& actor, float delta) {
    auto& monster = to_monster(actor);
    state_timer_ += delta;

    // Monster stays stationary during death animation
    monster.set_velocity_x(0.0f);

    // Check data-driven transitions (though death is usually final)
    check_transitions(actor, delta);
}
