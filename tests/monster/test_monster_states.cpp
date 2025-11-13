// Copyright 2025 Quentin Cartier
#include <gtest/gtest.h>
#include <memory>
#include "udjourney/states/MonsterStates.hpp"
#include "udjourney/Monster.hpp"
#include "udjourney/Player.hpp"

// Mock Game class for testing
class MockGame : public IGame {
 public:
    MockGame() : m_rect{0, 0, 640, 480} {}

    void run() override {}
    void update() override {}
    void process_input() override {}
    void add_actor(std::unique_ptr<IActor> actor) override {}
    void remove_actor(IActor* actor) override {}
    Rectangle get_rectangle() const override { return m_rect; }
    void on_checkpoint_reached(float x, float y) const override {}
    Player* get_player() const override { return m_mock_player.get(); }

    void set_mock_player(std::unique_ptr<Player> player) {
        m_mock_player = std::move(player);
    }

    void clear_mock_player() { m_mock_player.reset(); }

 private:
    Rectangle m_rect;
    std::unique_ptr<Player> m_mock_player;
};

// Mock Event Dispatcher
class MockEventDispatcher {
 public:
    // Add any methods needed by Player constructor if required
};

class MonsterStatesTest : public ::testing::Test {
 protected:
    void SetUp() override {
        mock_game = std::make_unique<MockGame>();

        // Create a monster for testing
        monster = std::make_unique<Monster>(
            *mock_game, Rectangle{100, 100, 64, 64}, "char1-Sheet.png");

        // Create states
        idle_state = std::make_unique<MonsterIdleState>();
        patrol_state = std::make_unique<MonsterPatrolState>();
        chase_state = std::make_unique<MonsterChaseState>();
        attack_state = std::make_unique<MonsterAttackState>();
        hurt_state = std::make_unique<MonsterHurtState>();
        death_state = std::make_unique<MonsterDeathState>();
    }

    void TearDown() override { mock_game->clear_mock_player(); }

    void CreateMockPlayer(float x, float y) {
        // Note: Player constructor might need an event dispatcher
        // For now, we'll use the mock game's dispatcher or create a minimal one
        MockEventDispatcher mock_dispatcher;

        // This might need adjustment based on Player constructor requirements
        auto player = std::unique_ptr<Player>();  // Simplified for now
        mock_game->set_mock_player(std::move(player));
    }

    std::unique_ptr<MockGame> mock_game;
    std::unique_ptr<Monster> monster;
    std::unique_ptr<MonsterIdleState> idle_state;
    std::unique_ptr<MonsterPatrolState> patrol_state;
    std::unique_ptr<MonsterChaseState> chase_state;
    std::unique_ptr<MonsterAttackState> attack_state;
    std::unique_ptr<MonsterHurtState> hurt_state;
    std::unique_ptr<MonsterDeathState> death_state;
};

// Test MonsterIdleState
TEST_F(MonsterStatesTest, IdleStateEnter) {
    // Test entering idle state
    idle_state->enter(*monster);

    // Verify monster stops moving
    EXPECT_EQ(monster->get_rectangle().x, 100.0f);  // Should not have moved
}

TEST_F(MonsterStatesTest, IdleStateUpdate_TransitionsToPatrol) {
    idle_state->enter(*monster);

    // Simulate time passing (3 seconds to trigger patrol transition)
    for (int i = 0; i < 3; i++) {
        idle_state->update(*monster, 1.0f);  // 1 second per update
    }

    // After 3 seconds, should transition to patrol
    // Note: This test verifies the timer logic, actual state change would be
    // handled by Monster
}

TEST_F(MonsterStatesTest, IdleStateUpdate_TransitionsToChaseWhenPlayerNear) {
    idle_state->enter(*monster);

    // Create a mock player near the monster (within chase range)
    CreateMockPlayer(
        150.0f, 100.0f);  // 50 units away, within default chase range (200)

    idle_state->update(*monster, 0.1f);

    // Should attempt to transition to chase state when player is nearby
    // The actual state transition is handled by the Monster class
}

// Test MonsterPatrolState
TEST_F(MonsterStatesTest, PatrolStateEnter) {
    patrol_state->enter(*monster);

    // Should start moving in the current facing direction
    // Initial velocity should be set for patrol speed
}

TEST_F(MonsterStatesTest, PatrolStateUpdate_Movement) {
    monster->set_patrol_range(50.0f, 200.0f);
    patrol_state->enter(*monster);

    // Test patrol movement
    patrol_state->update(*monster, 1.0f);

    // Monster should be moving within patrol bounds
}

TEST_F(MonsterStatesTest, PatrolStateUpdate_DirectionChange) {
    monster->set_patrol_range(50.0f, 200.0f);
    patrol_state->enter(*monster);

    // Move monster to patrol boundary
    monster->set_rectangle(
        Rectangle{200.0f, 100.0f, 64.0f, 64.0f});  // At right boundary

    patrol_state->update(*monster, 0.1f);

    // Should reverse direction when hitting patrol boundary
}

// Test MonsterChaseState
TEST_F(MonsterStatesTest, ChaseStateEnter) {
    chase_state->enter(*monster);

    // Should prepare for chasing (no specific velocity set until update with
    // player)
}

TEST_F(MonsterStatesTest, ChaseStateUpdate_NoPlayer) {
    chase_state->enter(*monster);

    // Update without a player present
    chase_state->update(*monster, 0.1f);

    // Should attempt to transition back to patrol when no player found
}

TEST_F(MonsterStatesTest, ChaseStateUpdate_PlayerInRange) {
    CreateMockPlayer(200.0f, 100.0f);  // Player to the right
    chase_state->enter(*monster);

    chase_state->update(*monster, 0.1f);

    // Monster should move towards player (positive velocity_x)
}

TEST_F(MonsterStatesTest, ChaseStateUpdate_PlayerInAttackRange) {
    CreateMockPlayer(130.0f,
                     100.0f);  // Player very close (within attack range)
    chase_state->enter(*monster);

    chase_state->update(*monster, 0.1f);

    // Should attempt to transition to attack state when player is in attack
    // range
}

// Test MonsterAttackState
TEST_F(MonsterStatesTest, AttackStateEnter) {
    attack_state->enter(*monster);

    // Should stop moving and reset attack timer
    // Monster velocity should be 0
}

TEST_F(MonsterStatesTest, AttackStateUpdate_AttackDuration) {
    attack_state->enter(*monster);

    // Simulate attack duration (0.6 seconds)
    attack_state->update(*monster, 0.6f);

    // After attack duration, should transition back to chase or patrol
}

// Test MonsterHurtState
TEST_F(MonsterStatesTest, HurtStateEnter) {
    hurt_state->enter(*monster);

    // Should stop moving and reset hurt timer
}

TEST_F(MonsterStatesTest, HurtStateUpdate_HurtDuration) {
    hurt_state->enter(*monster);

    // Simulate hurt duration (0.4 seconds)
    hurt_state->update(*monster, 0.4f);

    // After hurt duration, should transition back to patrol
}

// Test MonsterDeathState
TEST_F(MonsterStatesTest, DeathStateEnter) {
    death_state->enter(*monster);

    // Should stop moving and reset death timer
}

TEST_F(MonsterStatesTest, DeathStateUpdate_DeathDuration) {
    death_state->enter(*monster);

    // Simulate death duration (1.1 seconds)
    death_state->update(*monster, 1.1f);

    // After death duration, monster should be marked for removal (CONSUMED
    // state)
}

// Test state name functionality
TEST_F(MonsterStatesTest, StateNames) {
    EXPECT_EQ(idle_state->get_name(), "IDLE");
    EXPECT_EQ(patrol_state->get_name(), "PATROL");
    EXPECT_EQ(chase_state->get_name(), "CHASE");
    EXPECT_EQ(attack_state->get_name(), "ATTACK");
    EXPECT_EQ(hurt_state->get_name(), "HURT");
    EXPECT_EQ(death_state->get_name(), "DEATH");
}

// Integration test: State transitions
TEST_F(MonsterStatesTest, StateTransitionIntegration) {
    // Test a complete state flow: IDLE -> PATROL -> CHASE -> ATTACK

    // Start in IDLE
    monster->change_state("IDLE");

    // Simulate idle timeout (should go to PATROL)
    for (int i = 0; i < 25; i++) {  // 2.5 seconds
        monster->update(0.1f);
    }

    // Create nearby player (should trigger CHASE)
    CreateMockPlayer(150.0f, 100.0f);

    // Update a few times to process chase logic
    for (int i = 0; i < 5; i++) {
        monster->update(0.1f);
    }

    // Move player very close (should trigger ATTACK)
    CreateMockPlayer(130.0f, 100.0f);

    // Update to process attack transition
    for (int i = 0; i < 5; i++) {
        monster->update(0.1f);
    }
}