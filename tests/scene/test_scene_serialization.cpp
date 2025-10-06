// Copyright 2025 Quentin Cartier

#include <gtest/gtest.h>

#include <filesystem>

#include "udjourney/scene/Scene.hpp"

using namespace udjourney::scene;

class SceneSerializationTest : public ::testing::Test {
 protected:
    void SetUp() override {
        test_dir = std::filesystem::temp_directory_path() /
                   "scene_serialization_tests";
        std::filesystem::create_directories(test_dir);
    }

    void TearDown() override {
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

    Scene create_test_scene() {
        Scene scene;
        scene.set_name("Serialization Test Scene");
        scene.set_player_spawn(7, 14);

        // Add static platform
        PlatformData static_platform;
        static_platform.tile_x = 0;
        static_platform.tile_y = 16;
        static_platform.width_tiles = 6;
        static_platform.height_tiles = 2;
        static_platform.behavior_type = PlatformBehaviorType::Static;
        scene.add_platform(static_platform);

        // Add horizontal platform with parameters
        PlatformData horizontal_platform;
        horizontal_platform.tile_x = 8;
        horizontal_platform.tile_y = 14;
        horizontal_platform.width_tiles = 4;
        horizontal_platform.height_tiles = 1;
        horizontal_platform.behavior_type = PlatformBehaviorType::Horizontal;
        horizontal_platform.behavior_params["speed"] = 3.2f;
        horizontal_platform.behavior_params["range"] = 7.5f;
        scene.add_platform(horizontal_platform);

        // Add eight-turn platform
        PlatformData eight_turn_platform;
        eight_turn_platform.tile_x = 14;
        eight_turn_platform.tile_y = 12;
        eight_turn_platform.width_tiles = 2;
        eight_turn_platform.height_tiles = 1;
        eight_turn_platform.behavior_type =
            PlatformBehaviorType::EightTurnHorizontal;
        eight_turn_platform.behavior_params["speed"] = 1.4f;
        eight_turn_platform.behavior_params["amplitude"] = 4.8f;
        scene.add_platform(eight_turn_platform);

        // Add oscillating platform
        PlatformData oscillating_platform;
        oscillating_platform.tile_x = 18;
        oscillating_platform.tile_y = 10;
        oscillating_platform.width_tiles = 3;
        oscillating_platform.height_tiles = 1;
        oscillating_platform.behavior_type =
            PlatformBehaviorType::OscillatingSize;
        oscillating_platform.behavior_params["min_scale"] = 0.4f;
        oscillating_platform.behavior_params["max_scale"] = 1.6f;
        oscillating_platform.behavior_params["speed"] = 2.1f;
        scene.add_platform(oscillating_platform);

        // Add platform with spikes
        PlatformData spike_platform;
        spike_platform.tile_x = 22;
        spike_platform.tile_y = 8;
        spike_platform.width_tiles = 5;
        spike_platform.height_tiles = 1;
        spike_platform.behavior_type = PlatformBehaviorType::Static;
        spike_platform.features.push_back(PlatformFeatureType::Spikes);
        spike_platform.feature_params["damage"] = 2.7f;
        scene.add_platform(spike_platform);

        return scene;
    }

    void compare_scenes(const Scene& original, const Scene& loaded) {
        // Compare basic properties
        EXPECT_EQ(original.get_name(), loaded.get_name());

        auto original_spawn = original.get_player_spawn();
        auto loaded_spawn = loaded.get_player_spawn();
        EXPECT_EQ(original_spawn.tile_x, loaded_spawn.tile_x);
        EXPECT_EQ(original_spawn.tile_y, loaded_spawn.tile_y);

        // Compare platforms
        const auto& original_platforms = original.get_platforms();
        const auto& loaded_platforms = loaded.get_platforms();
        ASSERT_EQ(original_platforms.size(), loaded_platforms.size());

        for (size_t i = 0; i < original_platforms.size(); ++i) {
            const auto& orig = original_platforms[i];
            const auto& load = loaded_platforms[i];

            EXPECT_EQ(orig.tile_x, load.tile_x)
                << "Platform " << i << " tile_x mismatch";
            EXPECT_EQ(orig.tile_y, load.tile_y)
                << "Platform " << i << " tile_y mismatch";
            EXPECT_EQ(orig.width_tiles, load.width_tiles)
                << "Platform " << i << " width_tiles mismatch";
            EXPECT_EQ(orig.height_tiles, load.height_tiles)
                << "Platform " << i << " height_tiles mismatch";
            EXPECT_EQ(orig.behavior_type, load.behavior_type)
                << "Platform " << i << " behavior_type mismatch";

            // Compare behavior parameters
            EXPECT_EQ(orig.behavior_params.size(), load.behavior_params.size())
                << "Platform " << i << " behavior_params size mismatch";

            for (const auto& [key, value] : orig.behavior_params) {
                ASSERT_TRUE(load.behavior_params.count(key))
                    << "Platform " << i << " missing behavior param: " << key;
                EXPECT_FLOAT_EQ(value, load.behavior_params.at(key))
                    << "Platform " << i << " behavior param " << key
                    << " mismatch";
            }

            // Compare features
            EXPECT_EQ(orig.features.size(), load.features.size())
                << "Platform " << i << " features size mismatch";

            for (size_t f = 0; f < orig.features.size(); ++f) {
                EXPECT_EQ(orig.features[f], load.features[f])
                    << "Platform " << i << " feature " << f << " mismatch";
            }

            // Compare feature parameters
            EXPECT_EQ(orig.feature_params.size(), load.feature_params.size())
                << "Platform " << i << " feature_params size mismatch";

            for (const auto& [key, value] : orig.feature_params) {
                ASSERT_TRUE(load.feature_params.count(key))
                    << "Platform " << i << " missing feature param: " << key;
                EXPECT_FLOAT_EQ(value, load.feature_params.at(key))
                    << "Platform " << i << " feature param " << key
                    << " mismatch";
            }
        }
    }

    std::filesystem::path test_dir;
};

// Test basic save and load roundtrip
TEST_F(SceneSerializationTest, SaveLoadRoundtrip) {
    Scene original = create_test_scene();

    std::filesystem::path save_path = test_dir / "roundtrip_test.json";

    // Save scene
    ASSERT_TRUE(original.save_to_file(save_path.string()))
        << "Failed to save scene to " << save_path;

    // Verify file exists
    EXPECT_TRUE(std::filesystem::exists(save_path))
        << "Scene file was not created";

    // Load scene
    Scene loaded;
    ASSERT_TRUE(loaded.load_from_file(save_path.string()))
        << "Failed to load scene from " << save_path;

    // Compare scenes
    compare_scenes(original, loaded);
}

// Test save to invalid path
TEST_F(SceneSerializationTest, SaveToInvalidPath) {
    Scene scene = create_test_scene();

    // Try to save to invalid path (non-existent directory)
    std::string invalid_path = "/non_existent_dir/cannot_create.json";

    EXPECT_FALSE(scene.save_to_file(invalid_path))
        << "Should fail to save to invalid path";
}

// Test empty scene serialization
TEST_F(SceneSerializationTest, EmptySceneSerialization) {
    Scene empty_scene;

    std::filesystem::path save_path = test_dir / "empty_scene.json";

    // Save empty scene
    ASSERT_TRUE(empty_scene.save_to_file(save_path.string()));

    // Load it back
    Scene loaded_empty;
    ASSERT_TRUE(loaded_empty.load_from_file(save_path.string()));

    // Compare
    compare_scenes(empty_scene, loaded_empty);
}

// Test scene with only player spawn
TEST_F(SceneSerializationTest, PlayerSpawnOnlyScene) {
    Scene scene;
    scene.set_name("Spawn Only Scene");
    scene.set_player_spawn(42, 24);

    std::filesystem::path save_path = test_dir / "spawn_only.json";

    ASSERT_TRUE(scene.save_to_file(save_path.string()));

    Scene loaded;
    ASSERT_TRUE(loaded.load_from_file(save_path.string()));

    compare_scenes(scene, loaded);
}

// Test multiple save/load cycles
TEST_F(SceneSerializationTest, MultipleSaveLoadCycles) {
    Scene scene = create_test_scene();

    for (int cycle = 0; cycle < 5; ++cycle) {
        std::filesystem::path path =
            test_dir / ("cycle_" + std::to_string(cycle) + ".json");

        // Save
        ASSERT_TRUE(scene.save_to_file(path.string()))
            << "Failed to save on cycle " << cycle;

        // Load into new scene
        Scene loaded;
        ASSERT_TRUE(loaded.load_from_file(path.string()))
            << "Failed to load on cycle " << cycle;

        // Compare
        compare_scenes(scene, loaded);

        // Use loaded scene for next cycle
        scene = std::move(loaded);
    }
}
