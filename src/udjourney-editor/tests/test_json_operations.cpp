// Copyright 2025 Quentin Cartier
#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "udjourney-editor/Editor.hpp"
#include "udjourney-editor/Level.hpp"
#include "helpers/test_helpers.hpp"

class JSONOperationsTest : public ::testing::Test {
 protected:
    void SetUp() override {
        // Initialize editor in each test to avoid graphics context conflicts
    }

    void TearDown() override {
        // Clean up any temporary files
        for (const auto& path : temp_files) {
            if (std::filesystem::exists(path)) {
                std::filesystem::remove(path);
            }
        }
    }

    std::string createTempFile(const std::string& suffix = ".json") {
        std::string temp_path =
            std::filesystem::temp_directory_path() / ("test_" + suffix);
        temp_files.push_back(temp_path);
        return temp_path;
    }

    std::vector<std::string> temp_files;
};

TEST_F(JSONOperationsTest, PlatformLevelJSONExport) {
    Editor editor;
    editor.init();

    // Create a level with platforms and player spawn
    Level& level = editor.get_test_level();
    level.player_spawn_x = 10;
    level.player_spawn_y = 5;

    // Add test platforms
    EditorPlatform platform1;
    platform1.tile_x = 8;
    platform1.tile_y = 3;
    platform1.width_tiles = 4.0f;
    platform1.height_tiles = 1.0f;
    platform1.behavior_type = PlatformBehaviorType::Static;
    platform1.features.clear();

    EditorPlatform platform2;
    platform2.tile_x = 15;
    platform2.tile_y = 6;
    platform2.width_tiles = 3.0f;
    platform2.height_tiles = 1.0f;
    platform2.behavior_type = PlatformBehaviorType::Horizontal;
    platform2.features.push_back(PlatformFeatureType::Checkpoint);

    EditorPlatform platform3;
    platform3.tile_x = 5;
    platform3.tile_y = 9;
    platform3.width_tiles = 2.0f;
    platform3.height_tiles = 0.5f;
    platform3.behavior_type = PlatformBehaviorType::EightTurnHorizontal;
    platform3.features.push_back(PlatformFeatureType::Spikes);
    platform3.features.push_back(PlatformFeatureType::Checkpoint);

    level.platforms = {platform1, platform2, platform3};

    // Export to temporary file
    std::string temp_path = createTempFile("platform_export.json");
    editor.test_export_platform_level_json(temp_path);

    // Verify file was created
    ASSERT_TRUE(std::filesystem::exists(temp_path));

    // Parse and verify JSON content
    std::ifstream file(temp_path);
    nlohmann::json exported_json;
    file >> exported_json;

    // Player spawn is exported correctly
    EXPECT_TRUE(exported_json.contains("player_spawn"));
    EXPECT_EQ(exported_json["player_spawn"]["x"], 10);
    EXPECT_EQ(exported_json["player_spawn"]["y"], 5);

    // Platforms array is exported correctly
    EXPECT_TRUE(exported_json.contains("platforms"));
    EXPECT_TRUE(exported_json["platforms"].is_array());
    EXPECT_EQ(exported_json["platforms"].size(), 3);

    // First platform (static, no features) is correct
    auto& p1 = exported_json["platforms"][0];
    EXPECT_EQ(p1["x"], 8);
    EXPECT_EQ(p1["y"], 3);
    EXPECT_EQ(p1["width"], 4.0f);
    EXPECT_EQ(p1["height"], 1.0f);
    EXPECT_EQ(p1["behavior"], "static");
    EXPECT_FALSE(p1.contains("features"));

    // Second platform (horizontal, checkpoint) is correct
    auto& p2 = exported_json["platforms"][1];
    EXPECT_EQ(p2["x"], 15);
    EXPECT_EQ(p2["y"], 6);
    EXPECT_EQ(p2["width"], 3.0f);
    EXPECT_EQ(p2["height"], 1.0f);
    EXPECT_EQ(p2["behavior"], "horizontal");
    EXPECT_TRUE(p2.contains("features"));
    EXPECT_EQ(p2["features"].size(), 1);
    EXPECT_EQ(p2["features"][0], "checkpoint");

    // Third platform (eight_turn, multiple features) is correct
    auto& p3 = exported_json["platforms"][2];
    EXPECT_EQ(p3["x"], 5);
    EXPECT_EQ(p3["y"], 9);
    EXPECT_EQ(p3["width"], 2.0f);
    EXPECT_EQ(p3["height"], 0.5f);
    EXPECT_EQ(p3["behavior"], "eight_turn");
    EXPECT_TRUE(p3.contains("features"));
    EXPECT_EQ(p3["features"].size(), 2);

    // Features should contain both spikes and checkpoint
    bool has_spikes = false, has_checkpoint = false;
    for (const auto& feature : p3["features"]) {
        if (feature == "spikes") has_spikes = true;
        if (feature == "checkpoint") has_checkpoint = true;
    }
    EXPECT_TRUE(has_spikes);
    EXPECT_TRUE(has_checkpoint);
}

TEST_F(JSONOperationsTest, PlatformLevelJSONImport) {
    Editor editor;
    editor.init();

    // Create test JSON content
    nlohmann::json test_json = {{"name", "Test Level"},
                                {"player_spawn", {{"x", 12}, {"y", 8}}},
                                {"platforms",
                                 {{{"x", 5},
                                   {"y", 10},
                                   {"width", 3.5},
                                   {"height", 1.2},
                                   {"behavior", "static"}},
                                  {{"x", 20},
                                   {"y", 15},
                                   {"width", 2.0},
                                   {"height", 0.8},
                                   {"behavior", "horizontal"},
                                   {"features", {"spikes"}}},
                                  {{"x", 8},
                                   {"y", 25},
                                   {"width", 4.0},
                                   {"height", 1.0},
                                   {"behavior", "oscillating_size"},
                                   {"features", {"checkpoint", "spikes"}}}}}};

    // Write to temporary file
    std::string temp_path = create_temp_file(test_json.dump(2));

    // Import the file
    editor.test_import_platform_level_json(temp_path);

    Level& level = editor.get_test_level();

    // Player spawn is imported correctly
    EXPECT_EQ(level.player_spawn_x, 12);
    EXPECT_EQ(level.player_spawn_y, 8);

    // Correct number of platforms imported
    EXPECT_EQ(level.platforms.size(), 3);

    // First platform (static) imported correctly
    const auto& p1 = level.platforms[0];
    EXPECT_EQ(p1.tile_x, 5);
    EXPECT_EQ(p1.tile_y, 10);
    EXPECT_EQ(p1.width_tiles, 3.5f);
    EXPECT_EQ(p1.height_tiles, 1.2f);
    EXPECT_EQ(p1.behavior_type, PlatformBehaviorType::Static);
    EXPECT_TRUE(p1.features.empty());

    // Second platform (horizontal with spikes) imported correctly
    const auto& p2 = level.platforms[1];
    EXPECT_EQ(p2.tile_x, 20);
    EXPECT_EQ(p2.tile_y, 15);
    EXPECT_EQ(p2.width_tiles, 2.0f);
    EXPECT_EQ(p2.height_tiles, 0.8f);
    EXPECT_EQ(p2.behavior_type, PlatformBehaviorType::Horizontal);
    EXPECT_EQ(p2.features.size(), 1);
    EXPECT_EQ(p2.features[0], PlatformFeatureType::Spikes);

    // Third platform (oscillating with multiple features) imported correctly
    const auto& p3 = level.platforms[2];
    EXPECT_EQ(p3.tile_x, 8);
    EXPECT_EQ(p3.tile_y, 25);
    EXPECT_EQ(p3.width_tiles, 4.0f);
    EXPECT_EQ(p3.height_tiles, 1.0f);
    EXPECT_EQ(p3.behavior_type, PlatformBehaviorType::OscillatingSize);
    EXPECT_EQ(p3.features.size(), 2);

    // Check both features are present
    bool has_spikes =
        std::find(p3.features.begin(),
                  p3.features.end(),
                  PlatformFeatureType::Spikes) != p3.features.end();
    bool has_checkpoint =
        std::find(p3.features.begin(),
                  p3.features.end(),
                  PlatformFeatureType::Checkpoint) != p3.features.end();
    EXPECT_TRUE(has_spikes);
    EXPECT_TRUE(has_checkpoint);
}

TEST_F(JSONOperationsTest, PlatformLevelJSONRoundTrip) {
    Editor editor;
    editor.init();

    Editor editor2;
    editor2.init();

    // Create original level
    Level& level1 = editor.get_test_level();
    level1.player_spawn_x = 7;
    level1.player_spawn_y = 3;

    EditorPlatform original_platform;
    original_platform.tile_x = 12;
    original_platform.tile_y = 18;
    original_platform.width_tiles = 2.5f;
    original_platform.height_tiles = 1.5f;
    original_platform.behavior_type = PlatformBehaviorType::EightTurnHorizontal;
    original_platform.features.push_back(PlatformFeatureType::Checkpoint);

    level1.platforms = {original_platform};

    // Export to file
    std::string temp_path = createTempFile("roundtrip.json");
    editor.test_export_platform_level_json(temp_path);

    // Import with second editor
    editor2.test_import_platform_level_json(temp_path);

    Level& level2 = editor2.get_test_level();

    // Player spawn preserved in round trip
    EXPECT_EQ(level2.player_spawn_x, level1.player_spawn_x);
    EXPECT_EQ(level2.player_spawn_y, level1.player_spawn_y);

    // Platform data preserved in round trip
    EXPECT_EQ(level2.platforms.size(), 1);
    const auto& imported_platform = level2.platforms[0];

    EXPECT_EQ(imported_platform.tile_x, original_platform.tile_x);
    EXPECT_EQ(imported_platform.tile_y, original_platform.tile_y);
    EXPECT_EQ(imported_platform.width_tiles, original_platform.width_tiles);
    EXPECT_EQ(imported_platform.height_tiles, original_platform.height_tiles);
    EXPECT_EQ(imported_platform.behavior_type, original_platform.behavior_type);
    EXPECT_EQ(imported_platform.features, original_platform.features);
}

TEST_F(JSONOperationsTest, InvalidJSONHandling) {
    Editor editor;
    editor.init();

    // Missing file - should not crash when importing non-existent file
    EXPECT_NO_THROW(
        editor.test_import_platform_level_json("non_existent_file.json"));

    // Invalid JSON syntax
    std::string invalid_json = "{ invalid json syntax }";
    std::string temp_path = create_temp_file(invalid_json);
    EXPECT_NO_THROW(editor.test_import_platform_level_json(temp_path));

    // Missing required fields
    nlohmann::json incomplete_json = {
        {"platforms",
         {{{"x", 5}, {"y", 10}}}}  // Missing width, height, behavior
    };
    std::string temp_path2 = create_temp_file(incomplete_json.dump());
    EXPECT_NO_THROW(editor.test_import_platform_level_json(temp_path2));

    // Unknown behavior type
    nlohmann::json unknown_behavior_json = {
        {"platforms",
         {{{"x", 5},
           {"y", 10},
           {"width", 2.0},
           {"height", 1.0},
           {"behavior", "unknown_behavior_type"}}}}};
    std::string temp_path3 = create_temp_file(unknown_behavior_json.dump());
    EXPECT_NO_THROW(editor.test_import_platform_level_json(temp_path3));

    Level& level = editor.get_test_level();
    if (!level.platforms.empty()) {
        // Should default to static behavior for unknown types
        EXPECT_EQ(level.platforms[0].behavior_type,
                  PlatformBehaviorType::Static);
    }
}
