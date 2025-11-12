// Copyright 2025 Quentin Cartier
#include "udjourney/states/MonsterStates.hpp"
#include "udjourney/Monster.hpp"
#include "udjourney/Player.hpp"
#include <cmath>
#include <iostream>

// Helper function to cast IActor to Monster
static Monster& to_monster(IActor& actor) {
    return static_cast<Monster&>(actor);
}

// ===== MonsterIdleState =====

void MonsterIdleState::enter(IActor& actor) {
    auto& monster = to_monster(actor);
    monster.set_velocity_x(0.0f);
    idle_timer_ = 0.0f;
    std::cout << "Monster entered IDLE state\n";
}

void MonsterIdleState::handleInput(IActor& actor) {
    // Idle state doesn't handle input
}

void MonsterIdleState::update(IActor& actor, float delta) {
    auto& monster = to_monster(actor);

    idle_timer_ += delta;

    // Check for player proximity - transition to chase or patrol
    auto* player_ptr = monster.find_player();
    if (player_ptr) {
        float distance =
            std::abs(monster.get_rectangle().x - player_ptr->get_rectangle().x);

        if (distance < monster.get_chase_range()) {
            monster.change_state("CHASE");
            return;
        }
    }

    // After idle time, start patrolling
    if (idle_timer_ > 2.0f) {
        monster.change_state("PATROL");
    }
}

// ===== MonsterPatrolState =====

void MonsterPatrolState::enter(IActor& actor) {
    auto& monster = to_monster(actor);
    // Start moving in the current facing direction
    monster.set_velocity_x(monster.is_facing_right()
                               ? monster.get_patrol_speed()
                               : -monster.get_patrol_speed());
    std::cout << "Monster entered PATROL state\n";
}

void MonsterPatrolState::handleInput(IActor& actor) {
    // Patrol state doesn't handle input
}

void MonsterPatrolState::update(IActor& actor, float delta) {
    auto& monster = to_monster(actor);
    std::cout << "Monster patrolling at velocity X: "
              << monster.get_patrol_speed() << "\n";
    // Check for player proximity
    auto* player_ptr = monster.find_player();
    if (player_ptr) {
        float distance =
            std::abs(monster.get_rectangle().x - player_ptr->get_rectangle().x);

        std::cout << "Distance to player: " << distance
                  << " vs chase range: " << monster.get_chase_range() << "\n";
        if (distance < monster.get_chase_range()) {
            std::cout << "Monster spotted player, switching to CHASE state\n";
            monster.change_state("CHASE");
            return;
        }
    } else {
        std::cout << "No player found by monster\n";
    }

    // Check patrol boundaries
    auto rect = monster.get_rectangle();
    if (monster.is_facing_right() &&
        rect.x >= monster.get_patrol_right_bound()) {
        monster.reverse_direction();
    } else if (!monster.is_facing_right() &&
               rect.x <= monster.get_patrol_left_bound()) {
        monster.reverse_direction();
    }
}

// ===== MonsterChaseState =====

void MonsterChaseState::enter(IActor& actor) {
    auto& monster = to_monster(actor);
    std::cout << "Monster entered CHASE state\n";
}

void MonsterChaseState::handleInput(IActor& actor) {
    // Chase state doesn't handle input
}

void MonsterChaseState::update(IActor& actor, float delta) {
    auto& monster = to_monster(actor);

    auto* player_ptr = monster.find_player();
    if (!player_ptr) {
        // Lost the player, return to patrol
        monster.change_state("PATROL");
        return;
    }

    float distance =
        std::abs(monster.get_rectangle().x - player_ptr->get_rectangle().x);

    // If player is in attack range, attack
    if (distance < monster.get_attack_range()) {
        monster.change_state("ATTACK");
        return;
    }

    // If player is too far, return to patrol
    if (distance > monster.get_chase_range()) {
        monster.change_state("PATROL");
        return;
    }

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

// ===== MonsterAttackState =====

void MonsterAttackState::enter(IActor& actor) {
    auto& monster = to_monster(actor);
    monster.set_velocity_x(0.0f);
    attack_timer_ = 0.0f;
    std::cout << "Monster entered ATTACK state\n";
}

void MonsterAttackState::handleInput(IActor& actor) {
    // Attack state doesn't handle input
}

void MonsterAttackState::update(IActor& actor, float delta) {
    auto& monster = to_monster(actor);

    attack_timer_ += delta;

    // Check if still in range
    auto* player_ptr = monster.find_player();
    if (!player_ptr) {
        monster.change_state("PATROL");
        return;
    }

    float distance =
        std::abs(monster.get_rectangle().x - player_ptr->get_rectangle().x);

    // Attack animation duration is ~0.5 seconds
    if (attack_timer_ > 0.5f) {
        // After attack, check if player is still close
        if (distance < monster.get_chase_range()) {
            monster.change_state("CHASE");
        } else {
            monster.change_state("PATROL");
        }
        attack_timer_ = 0.0f;
    }
}

// ===== MonsterHurtState =====

void MonsterHurtState::enter(IActor& actor) {
    auto& monster = to_monster(actor);
    monster.set_velocity_x(0.0f);
    hurt_timer_ = 0.0f;
    std::cout << "Monster entered HURT state\n";
}

void MonsterHurtState::handleInput(IActor& actor) {
    // Hurt state doesn't handle input
}

void MonsterHurtState::update(IActor& actor, float delta) {
    auto& monster = to_monster(actor);

    hurt_timer_ += delta;

    // Hurt animation lasts ~0.3 seconds
    if (hurt_timer_ > 0.3f) {
        // Check health and transition appropriately
        // For now, just go back to patrol (health system not implemented yet)
        monster.change_state("PATROL");
    }
}

// ===== MonsterDeathState =====

void MonsterDeathState::enter(IActor& actor) {
    auto& monster = to_monster(actor);
    monster.set_velocity_x(0.0f);
    death_timer_ = 0.0f;
    std::cout << "Monster entered DEATH state\n";
}

void MonsterDeathState::handleInput(IActor& actor) {
    // Death state doesn't handle input
}

void MonsterDeathState::update(IActor& actor, float delta) {
    auto& monster = to_monster(actor);

    death_timer_ += delta;

    // Death animation lasts ~1 second, then mark for removal
    if (death_timer_ > 1.0f) {
        monster.set_state(ActorState::CONSUMED);
    }
}
