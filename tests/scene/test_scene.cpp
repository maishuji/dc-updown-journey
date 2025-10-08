// Copyright 2025 Quentin Cartier

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "udjourney/scene/Scene.hpp"

using namespace udjourney::scene;

class SceneTest : public ::testing::Test {
 protected:
    void SetUp() override {
        // Create temporary test directory
        test_dir = std::filesystem::temp_directory_path() / "scene_tests";
        std::filesystem::create_directories(test_dir);

        // Create a simple test scene file
        simple_scene_path = test_dir / "simple_scene.json";
        create_simple_test_scene();

        // Create complex scene file
        complex_scene_path = test_dir / "complex_scene.json";
        create_complex_test_scene();
    }

    void TearDown() override {
        // Clean up test files
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

    void create_simple_test_scene() {
        std::ofstream file(simple_scene_path);
        file << R"({
            "name": "Simple Test Scene",
            "player_spawn": {
                "x": 2,
                "y": 8
            },
            "platforms": [
                {
                    "x": 0,
                    "y": 10,
                    "width": 5,
                    "height": 1,
                    "behavior": "static"
                }
            ]
        })";
    }

    void create_complex_test_scene() {
        std::ofstream file(complex_scene_path);
        file << R"({
            "name": "Complex Test Scene",
            "player_spawn": {
                "x": 1,
                "y": 12
            },
            "platforms": [
                {
                    "x": 0,
                    "y": 15,
                    "width": 3,
                    "height": 2,
                    "behavior": "horizontal",
                    "behavior_params": {
                        "speed": 2.5,
                        "range": 5.0
                    }
                },
                {
                    "x": 8,
                    "y": 13,
                    "width": 2,
                    "height": 1,
                    "behavior": "eight_turn",
                    "behavior_params": {
                        "speed": 1.8,
                        "amplitude": 3.5
                    }
                },
                {
                    "x": 12,
                    "y": 11,
                    "width": 4,
                    "height": 1,
                    "behavior": "static",
                    "features": ["spikes"],
                    "feature_params": {
                        "damage": 1.5
                    }
                }
            ]
        })";
    }

    std::filesystem::path test_dir;
    std::filesystem::path simple_scene_path;
    std::filesystem::path complex_scene_path;
};

// Test default constructor
TEST_F(SceneTest, DefaultConstructor) {
    Scene scene;

    EXPECT_EQ(scene.get_name(), "Unnamed Level");
    EXPECT_EQ(scene.get_platforms().size(), 0);

    auto spawn = scene.get_player_spawn();
    EXPECT_EQ(spawn.tile_x, 0);
    EXPECT_EQ(spawn.tile_y, 0);
}

// Test basic scene loading
TEST_F(SceneTest, LoadSimpleScene) {
    Scene scene;

    ASSERT_TRUE(scene.load_from_file(simple_scene_path.string()));

    EXPECT_EQ(scene.get_name(), "Simple Test Scene");

    auto spawn = scene.get_player_spawn();
    EXPECT_EQ(spawn.tile_x, 2);
    EXPECT_EQ(spawn.tile_y, 8);

    const auto& platforms = scene.get_platforms();
    ASSERT_EQ(platforms.size(), 1);

    const auto& platform = platforms[0];
    EXPECT_EQ(platform.tile_x, 0);
    EXPECT_EQ(platform.tile_y, 10);
    EXPECT_EQ(platform.width_tiles, 5);
    EXPECT_EQ(platform.height_tiles, 1);
    EXPECT_EQ(platform.behavior_type, PlatformBehaviorType::Static);
}

// Test complex scene loading
TEST_F(SceneTest, LoadComplexScene) {
    Scene scene;

    ASSERT_TRUE(scene.load_from_file(complex_scene_path.string()));

    EXPECT_EQ(scene.get_name(), "Complex Test Scene");

    const auto& platforms = scene.get_platforms();
    ASSERT_EQ(platforms.size(), 3);

    // Test horizontal platform
    const auto& horizontal_platform = platforms[0];
    EXPECT_EQ(horizontal_platform.behavior_type,
              PlatformBehaviorType::Horizontal);
    EXPECT_EQ(horizontal_platform.behavior_params.at("speed"), 2.5f);
    EXPECT_EQ(horizontal_platform.behavior_params.at("range"), 5.0f);

    // Test eight-turn platform
    const auto& eight_turn_platform = platforms[1];
    EXPECT_EQ(eight_turn_platform.behavior_type,
              PlatformBehaviorType::EightTurnHorizontal);
    EXPECT_EQ(eight_turn_platform.behavior_params.at("speed"), 1.8f);
    EXPECT_EQ(eight_turn_platform.behavior_params.at("amplitude"), 3.5f);

    // Test platform with spikes
    const auto& spike_platform = platforms[2];
    EXPECT_EQ(spike_platform.behavior_type, PlatformBehaviorType::Static);
    ASSERT_EQ(spike_platform.features.size(), 1);
    EXPECT_EQ(spike_platform.features[0], PlatformFeatureType::Spikes);
    EXPECT_EQ(spike_platform.feature_params.at("damage"), 1.5f);
}

// Test file constructor
TEST_F(SceneTest, FileConstructor) {
    Scene scene(simple_scene_path.string());

    EXPECT_EQ(scene.get_name(), "Simple Test Scene");
    EXPECT_EQ(scene.get_platforms().size(), 1);
}

// Test loading non-existent file
TEST_F(SceneTest, LoadNonExistentFile) {
    Scene scene;

    EXPECT_FALSE(scene.load_from_file("non_existent_file.json"));

    // Scene should remain unchanged
    EXPECT_EQ(scene.get_name(), "Unnamed Level");
    EXPECT_EQ(scene.get_platforms().size(), 0);
}

// Test setters and getters
TEST_F(SceneTest, SettersAndGetters) {
    Scene scene;

    scene.set_name("Test Scene Name");
    EXPECT_EQ(scene.get_name(), "Test Scene Name");

    scene.set_player_spawn(10, 15);
    auto spawn = scene.get_player_spawn();
    EXPECT_EQ(spawn.tile_x, 10);
    EXPECT_EQ(spawn.tile_y, 15);

    PlatformData platform;
    platform.tile_x = 5;
    platform.tile_y = 8;
    platform.width_tiles = 3;
    platform.height_tiles = 2;
    platform.behavior_type = PlatformBehaviorType::Horizontal;

    scene.add_platform(platform);

    const auto& platforms = scene.get_platforms();
    ASSERT_EQ(platforms.size(), 1);

    const auto& added_platform = platforms[0];
    EXPECT_EQ(added_platform.tile_x, 5);
    EXPECT_EQ(added_platform.tile_y, 8);
    EXPECT_EQ(added_platform.width_tiles, 3);
    EXPECT_EQ(added_platform.height_tiles, 2);
    EXPECT_EQ(added_platform.behavior_type, PlatformBehaviorType::Horizontal);
}

// Test all behavior types parsing
TEST_F(SceneTest, AllBehaviorTypesParsing) {
    // Create a scene with all behavior types
    std::filesystem::path all_behaviors_path = test_dir / "all_behaviors.json";
    std::ofstream file(all_behaviors_path);
    file << R"({
        "name": "All Behaviors Scene",
        "player_spawn": {"x": 0, "y": 0},
        "platforms": [
            {"x": 0, "y": 0, "width": 1, "height": 1, "behavior": "static"},
            {"x": 1, "y": 0, "width": 1, "height": 1, "behavior": "horizontal"},
            {"x": 2, "y": 0, "width": 1, "height": 1, "behavior": "eight_turn"},
            {"x": 3, "y": 0, "width": 1, "height": 1, "behavior": "oscillating_size"},
            {"x": 4, "y": 0, "width": 1, "height": 1, "behavior": "unknown_behavior"}
        ]
    })";
    file.close();

    Scene scene;
    ASSERT_TRUE(scene.load_from_file(all_behaviors_path.string()));

    const auto& platforms = scene.get_platforms();
    ASSERT_EQ(platforms.size(), 5);

    EXPECT_EQ(platforms[0].behavior_type, PlatformBehaviorType::Static);
    EXPECT_EQ(platforms[1].behavior_type, PlatformBehaviorType::Horizontal);
    EXPECT_EQ(platforms[2].behavior_type,
              PlatformBehaviorType::EightTurnHorizontal);
    EXPECT_EQ(platforms[3].behavior_type,
              PlatformBehaviorType::OscillatingSize);
    EXPECT_EQ(platforms[4].behavior_type,
              PlatformBehaviorType::Static);  // Unknown defaults to static
}
