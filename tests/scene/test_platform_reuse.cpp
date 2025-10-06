// Copyright 2025 Quentin Cartier

#include <gtest/gtest.h>
#include <memory>

#include "udjourney/platform/Platform.hpp"
#include "udjourney/platform/reuse_strategies/RandomizePositionStrategy.hpp"
#include "udjourney/platform/reuse_strategies/NoReuseStrategy.hpp"
#include "udjourney/interfaces/IGame.hpp"

// Mock Game class for testing
class MockGame : public IGame {
 public:
    MockGame() : m_rect({0, 0, 640, 480}) {}
    
    Rectangle get_rectangle() const override { return m_rect; }
    
    // Mock implementations of pure virtual methods
    void run() override {}
    void update() override {}
    void process_input() override {}
    void add_actor(std::unique_ptr<IActor> actor) override {}
    void remove_actor(IActor *actor) override {}
    
 private:
    Rectangle m_rect;
};

class PlatformReuseTest : public ::testing::Test {
 protected:
    void SetUp() override {
        // Create a minimal mock game for testing
        game = std::make_unique<MockGame>();
    }

    std::unique_ptr<MockGame> game;
};

// Test that scene-based platforms (no reuse strategy) mark themselves as CONSUMED
TEST_F(PlatformReuseTest, ScenePlatformNoReuse) {
    Rectangle test_rect = {100, 100, 50, 20};
    
    // Create a platform without reuse strategy (like scene-based platforms)
    auto platform = std::make_unique<Platform>(*game, test_rect, BLUE, false, nullptr);
    
    // Initially, platform should be ONGOING
    EXPECT_EQ(platform->get_state(), ActorState::ONGOING);
    EXPECT_FALSE(platform->has_reuse_strategy());
    
    // When reused, it should mark itself as CONSUMED (ready for removal)
    platform->reuse();
    
    EXPECT_EQ(platform->get_state(), ActorState::CONSUMED);
}

// Test that random platforms with RandomizePositionStrategy get repositioned
TEST_F(PlatformReuseTest, RandomPlatformReuse) {
    Rectangle test_rect = {100, 100, 50, 20};
    
    // Create a platform with RandomizePositionStrategy (like random platforms)
    auto platform = std::make_unique<Platform>(*game, test_rect, BLUE, false, 
                                              std::make_unique<RandomizePositionStrategy>());
    
    // Initially, platform should be ONGOING
    EXPECT_EQ(platform->get_state(), ActorState::ONGOING);
    EXPECT_TRUE(platform->has_reuse_strategy());
    
    Rectangle original_rect = platform->get_rectangle();
    
    // Mark platform as consumed (simulating going out of bounds)
    platform->set_state(ActorState::CONSUMED);
    
    // When reused, it should be repositioned and state reset to ONGOING
    platform->reuse();
    
    EXPECT_EQ(platform->get_state(), ActorState::ONGOING);
    
    Rectangle new_rect = platform->get_rectangle();
    
    // The Y position should be different (moved to bottom of screen)
    // X position might be different due to randomization
    EXPECT_NE(original_rect.y, new_rect.y);
}

// Test NoReuseStrategy explicitly
TEST_F(PlatformReuseTest, NoReuseStrategyBehavior) {
    Rectangle test_rect = {100, 100, 50, 20};
    
    // Create a platform with explicit NoReuseStrategy
    auto platform = std::make_unique<Platform>(*game, test_rect, BLUE, false, 
                                              std::make_unique<NoReuseStrategy>());
    
    // Initially, platform should be ONGOING
    EXPECT_EQ(platform->get_state(), ActorState::ONGOING);
    EXPECT_TRUE(platform->has_reuse_strategy());
    
    // When reused, NoReuseStrategy should mark it as CONSUMED
    platform->reuse();
    
    EXPECT_EQ(platform->get_state(), ActorState::CONSUMED);
}

// Test platform reuse strategy setter
TEST_F(PlatformReuseTest, SetReuseStrategy) {
    Rectangle test_rect = {100, 100, 50, 20};
    
    // Create a platform without reuse strategy
    auto platform = std::make_unique<Platform>(*game, test_rect);
    
    EXPECT_FALSE(platform->has_reuse_strategy());
    
    // Set a reuse strategy
    platform->set_reuse_strategy(std::make_unique<RandomizePositionStrategy>());
    
    EXPECT_TRUE(platform->has_reuse_strategy());
}
