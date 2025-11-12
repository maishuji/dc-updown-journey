// Copyright 2025 Quentin Cartier
#include <gtest/gtest.h>
#include <memory>
#include "udjourney/states/MonsterStates.hpp"

// Simple mock for Player that doesn't inherit from the complex Player class
class SimplePlayer {
 public:
    SimplePlayer(float x, float y) : x_(x), y_(y) {}
    float get_x() const { return x_; }
    float get_y() const { return y_; }
    Rectangle get_rectangle() const { return {x_, y_, 20, 20}; }

 private:
    float x_, y_;
};

// Mock Actor class for testing states without complex dependencies
class MockMonsterActor : public IActor {
 public:
    MockMonsterActor() :
        IActor(mock_game),
        position_x_(100.0f),
        velocity_x_(0.0f),
        facing_right_(true),
        chase_range_(200.0f),
        attack_range_(50.0f),
        patrol_left_(50.0f),
        patrol_right_(250.0f),
        patrol_speed_(50.0f),
        chase_speed_(75.0f),
        state_change_requested_("") {}

    // IActor interface implementation
    void draw() const override {}
    void update(float delta) override { position_x_ += velocity_x_ * delta; }
    void process_input() override {}
    void set_rectangle(Rectangle rect) override { rect_ = rect; }
    Rectangle get_rectangle() const override {
        rect_.x = position_x_;
        return rect_;
    }
    bool check_collision(const IActor& other) const override { return false; }
    constexpr uint8_t get_group_id() const override { return 2; }

    // Mock Monster interface for states
    void set_velocity_x(float vx) { velocity_x_ = vx; }
    float get_chase_range() const { return chase_range_; }
    float get_attack_range() const { return attack_range_; }
    float get_patrol_left_bound() const { return patrol_left_; }
    float get_patrol_right_bound() const { return patrol_right_; }
    float get_patrol_speed() const { return patrol_speed_; }
    float get_chase_speed() const { return chase_speed_; }
    bool is_facing_right() const { return facing_right_; }
    void set_facing_right(bool right) { facing_right_ = right; }
    void reverse_direction() {
        facing_right_ = !facing_right_;
        velocity_x_ = -velocity_x_;
    }

    void change_state(const std::string& new_state) {
        state_change_requested_ = new_state;
    }

    Player* find_player() const {
        // Cast SimplePlayer to Player* for compatibility
        return reinterpret_cast<Player*>(simple_player_);
    }

    void set_simple_player(SimplePlayer* player) { simple_player_ = player; }

    // Test accessors
    float get_position_x() const { return position_x_; }
    float get_velocity_x() const { return velocity_x_; }
    std::string get_state_change_requested() const {
        return state_change_requested_;
    }
    void clear_state_change_request() { state_change_requested_ = ""; }

 private:
    // Mock game that returns basic rectangle
    class MockGame : public IGame {
     public:
        void run() override {}
        void update() override {}
        void process_input() override {}
        void add_actor(std::unique_ptr<IActor> actor) override {}
        void remove_actor(IActor* actor) override {}
        Rectangle get_rectangle() const override { return {0, 0, 640, 480}; }
        void on_checkpoint_reached(float x, float y) const override {}
        Player* get_player() const override { return nullptr; }
    };

    static MockGame mock_game;
    mutable Rectangle rect_{100, 100, 64, 64};
    float position_x_;
    float velocity_x_;
    bool facing_right_;
    float chase_range_;
    float attack_range_;
    float patrol_left_;
    float patrol_right_;
    float patrol_speed_;
    float chase_speed_;
    std::string state_change_requested_;
    SimplePlayer* simple_player_ = nullptr;
};

// Static member definition
MockMonsterActor::MockGame MockMonsterActor::mock_game;

class MonsterStatesTest : public ::testing::Test {
 protected:
    void SetUp() override {
        mock_actor = std::make_unique<MockMonsterActor>();

        idle_state = std::make_unique<MonsterIdleState>();
        patrol_state = std::make_unique<MonsterPatrolState>();
        chase_state = std::make_unique<MonsterChaseState>();
        attack_state = std::make_unique<MonsterAttackState>();
        hurt_state = std::make_unique<MonsterHurtState>();
        death_state = std::make_unique<MonsterDeathState>();
    }

    std::unique_ptr<MockMonsterActor> mock_actor;
    std::unique_ptr<MonsterIdleState> idle_state;
    std::unique_ptr<MonsterPatrolState> patrol_state;
    std::unique_ptr<MonsterChaseState> chase_state;
    std::unique_ptr<MonsterAttackState> attack_state;
    std::unique_ptr<MonsterHurtState> hurt_state;
    std::unique_ptr<MonsterDeathState> death_state;
};

// Test state names
TEST_F(MonsterStatesTest, StateNames) {
    EXPECT_EQ(idle_state->get_name(), "IDLE");
    EXPECT_EQ(patrol_state->get_name(), "PATROL");
    EXPECT_EQ(chase_state->get_name(), "CHASE");
    EXPECT_EQ(attack_state->get_name(), "ATTACK");
    EXPECT_EQ(hurt_state->get_name(), "HURT");
    EXPECT_EQ(death_state->get_name(), "DEATH");
}

// Test IdleState
TEST_F(MonsterStatesTest, IdleState_Enter_StopsMovement) {
    mock_actor->set_velocity_x(100.0f);  // Set some initial velocity

    idle_state->enter(*mock_actor);

    EXPECT_EQ(mock_actor->get_velocity_x(), 0.0f);
}

TEST_F(MonsterStatesTest, IdleState_Update_TransitionsToPatrolAfterTime) {
    idle_state->enter(*mock_actor);
    mock_actor->clear_state_change_request();

    // Update for more than 2 seconds (idle timeout)
    idle_state->update(*mock_actor, 2.5f);

    EXPECT_EQ(mock_actor->get_state_change_requested(), "PATROL");
}

TEST_F(MonsterStatesTest, IdleState_Update_TransitionsToChaseWhenPlayerNear) {
    // Create mock player within chase range
    auto mock_player =
        std::make_unique<MockPlayer>(150.0f, 100.0f);  // Within chase range
    mock_actor->set_mock_player(mock_player.get());

    idle_state->enter(*mock_actor);
    mock_actor->clear_state_change_request();

    idle_state->update(*mock_actor, 0.1f);

    EXPECT_EQ(mock_actor->get_state_change_requested(), "CHASE");
}

// Test PatrolState
TEST_F(MonsterStatesTest, PatrolState_Enter_StartsMovement) {
    patrol_state->enter(*mock_actor);

    // Should set velocity based on facing direction and patrol speed
    float expected_velocity = mock_actor->is_facing_right()
                                  ? mock_actor->get_patrol_speed()
                                  : -mock_actor->get_patrol_speed();
    EXPECT_EQ(mock_actor->get_velocity_x(), expected_velocity);
}

TEST_F(MonsterStatesTest, PatrolState_Update_TransitionsToChaseWhenPlayerNear) {
    auto mock_player = std::make_unique<MockPlayer>(150.0f, 100.0f);
    mock_actor->set_mock_player(mock_player.get());

    patrol_state->enter(*mock_actor);
    mock_actor->clear_state_change_request();

    patrol_state->update(*mock_actor, 0.1f);

    EXPECT_EQ(mock_actor->get_state_change_requested(), "CHASE");
}

// Test ChaseState
TEST_F(MonsterStatesTest, ChaseState_Update_NoPlayer_TransitionsToPatrol) {
    mock_actor->set_mock_player(nullptr);

    chase_state->enter(*mock_actor);
    mock_actor->clear_state_change_request();

    chase_state->update(*mock_actor, 0.1f);

    EXPECT_EQ(mock_actor->get_state_change_requested(), "PATROL");
}

TEST_F(MonsterStatesTest,
       ChaseState_Update_PlayerInAttackRange_TransitionsToAttack) {
    // Player very close (within attack range)
    auto mock_player = std::make_unique<MockPlayer>(130.0f, 100.0f);
    mock_actor->set_mock_player(mock_player.get());

    chase_state->enter(*mock_actor);
    mock_actor->clear_state_change_request();

    chase_state->update(*mock_actor, 0.1f);

    EXPECT_EQ(mock_actor->get_state_change_requested(), "ATTACK");
}

TEST_F(MonsterStatesTest, ChaseState_Update_PlayerTooFar_TransitionsToPatrol) {
    // Player very far (outside chase range)
    auto mock_player = std::make_unique<MockPlayer>(400.0f, 100.0f);
    mock_actor->set_mock_player(mock_player.get());

    chase_state->enter(*mock_actor);
    mock_actor->clear_state_change_request();

    chase_state->update(*mock_actor, 0.1f);

    EXPECT_EQ(mock_actor->get_state_change_requested(), "PATROL");
}

TEST_F(MonsterStatesTest, ChaseState_Update_MovesTowardPlayer) {
    // Player to the right
    auto mock_player = std::make_unique<MockPlayer>(200.0f, 100.0f);
    mock_actor->set_mock_player(mock_player.get());

    chase_state->enter(*mock_actor);

    chase_state->update(*mock_actor, 0.1f);

    // Should move right (positive velocity) and face right
    EXPECT_GT(mock_actor->get_velocity_x(), 0.0f);
    EXPECT_TRUE(mock_actor->is_facing_right());

    // Test player to the left
    mock_player = std::make_unique<MockPlayer>(50.0f, 100.0f);
    mock_actor->set_mock_player(mock_player.get());

    chase_state->update(*mock_actor, 0.1f);

    // Should move left (negative velocity) and face left
    EXPECT_LT(mock_actor->get_velocity_x(), 0.0f);
    EXPECT_FALSE(mock_actor->is_facing_right());
}

// Test AttackState
TEST_F(MonsterStatesTest, AttackState_Enter_StopsMovement) {
    mock_actor->set_velocity_x(100.0f);

    attack_state->enter(*mock_actor);

    EXPECT_EQ(mock_actor->get_velocity_x(), 0.0f);
}

TEST_F(MonsterStatesTest, AttackState_Update_TransitionsAfterDuration) {
    attack_state->enter(*mock_actor);
    mock_actor->clear_state_change_request();

    // Attack duration is ~0.5 seconds
    attack_state->update(*mock_actor, 0.6f);

    // Should transition to chase or patrol after attack duration
    EXPECT_FALSE(mock_actor->get_state_change_requested().empty());
}

// Test HurtState
TEST_F(MonsterStatesTest, HurtState_Enter_StopsMovement) {
    mock_actor->set_velocity_x(100.0f);

    hurt_state->enter(*mock_actor);

    EXPECT_EQ(mock_actor->get_velocity_x(), 0.0f);
}

TEST_F(MonsterStatesTest, HurtState_Update_TransitionsToPatrolAfterDuration) {
    hurt_state->enter(*mock_actor);
    mock_actor->clear_state_change_request();

    // Hurt duration is ~0.3 seconds
    hurt_state->update(*mock_actor, 0.4f);

    EXPECT_EQ(mock_actor->get_state_change_requested(), "PATROL");
}

// Test DeathState
TEST_F(MonsterStatesTest, DeathState_Enter_StopsMovement) {
    mock_actor->set_velocity_x(100.0f);

    death_state->enter(*mock_actor);

    EXPECT_EQ(mock_actor->get_velocity_x(), 0.0f);
}

TEST_F(MonsterStatesTest, DeathState_Update_MarksForRemovalAfterDuration) {
    death_state->enter(*mock_actor);

    // Death duration is ~1 second
    death_state->update(*mock_actor, 1.1f);

    // Should mark actor as CONSUMED for removal
    EXPECT_EQ(mock_actor->get_state(), ActorState::CONSUMED);
}