// Copyright 2025 Quentin Cartier
#include <filesystem>
#include <fstream>
#include <string>

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include "udjourney-editor/Editor.hpp"
#include "udjourney-editor/Level.hpp"
#include "test_helpers.hpp"

TEST_CASE("Platform Level JSON Export", "[json][export][platform]") {
    Editor editor;
    editor.init();  // Initialize editor

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
    std::string temp_path =
        std::filesystem::temp_directory_path() / "test_platform_export.json";
    editor.test_export_platform_level_json(temp_path);

    // Verify file was created
    REQUIRE(std::filesystem::exists(temp_path));

    // Parse and verify JSON content
    std::ifstream file(temp_path);
    nlohmann::json exported_json;
    file >> exported_json;

    SECTION("Player spawn is exported correctly") {
        REQUIRE(exported_json.contains("player_spawn"));
        REQUIRE(exported_json["player_spawn"]["x"] == 10);
        REQUIRE(exported_json["player_spawn"]["y"] == 5);
    }

    SECTION("Platforms array is exported correctly") {
        REQUIRE(exported_json.contains("platforms"));
        REQUIRE(exported_json["platforms"].is_array());
        REQUIRE(exported_json["platforms"].size() == 3);
    }

    SECTION("First platform (static, no features) is correct") {
        auto& p1 = exported_json["platforms"][0];
        REQUIRE(p1["x"] == 8);
        REQUIRE(p1["y"] == 3);
        REQUIRE(p1["width"] == 4.0f);
        REQUIRE(p1["height"] == 1.0f);
        REQUIRE(p1["behavior"] == "static");
        REQUIRE_FALSE(p1.contains("features"));
    }

    SECTION("Second platform (horizontal, checkpoint) is correct") {
        auto& p2 = exported_json["platforms"][1];
        REQUIRE(p2["x"] == 15);
        REQUIRE(p2["y"] == 6);
        REQUIRE(p2["width"] == 3.0f);
        REQUIRE(p2["height"] == 1.0f);
        REQUIRE(p2["behavior"] == "horizontal");
        REQUIRE(p2.contains("features"));
        REQUIRE(p2["features"].size() == 1);
        REQUIRE(p2["features"][0] == "checkpoint");
    }

    SECTION("Third platform (eight_turn, multiple features) is correct") {
        auto& p3 = exported_json["platforms"][2];
        REQUIRE(p3["x"] == 5);
        REQUIRE(p3["y"] == 9);
        REQUIRE(p3["width"] == 2.0f);
        REQUIRE(p3["height"] == 0.5f);
        REQUIRE(p3["behavior"] == "eight_turn");
        REQUIRE(p3.contains("features"));
        REQUIRE(p3["features"].size() == 2);

        // Features should contain both spikes and checkpoint
        bool has_spikes = false, has_checkpoint = false;
        for (const auto& feature : p3["features"]) {
            if (feature == "spikes") has_spikes = true;
            if (feature == "checkpoint") has_checkpoint = true;
        }
        REQUIRE(has_spikes);
        REQUIRE(has_checkpoint);
    }

    // Clean up
    std::filesystem::remove(temp_path);
}

TEST_CASE("Platform Level JSON Import", "[json][import][platform]") {
    Editor editor;
    editor.init();  // Initialize editor

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

    SECTION("Player spawn is imported correctly") {
        REQUIRE(level.player_spawn_x == 12);
        REQUIRE(level.player_spawn_y == 8);
    }

    SECTION("Correct number of platforms imported") {
        REQUIRE(level.platforms.size() == 3);
    }

    SECTION("First platform (static) imported correctly") {
        const auto& p1 = level.platforms[0];
        REQUIRE(p1.tile_x == 5);
        REQUIRE(p1.tile_y == 10);
        REQUIRE(p1.width_tiles == 3.5f);
        REQUIRE(p1.height_tiles == 1.2f);
        REQUIRE(p1.behavior_type == PlatformBehaviorType::Static);
        REQUIRE(p1.features.empty());
    }

    SECTION("Second platform (horizontal with spikes) imported correctly") {
        const auto& p2 = level.platforms[1];
        REQUIRE(p2.tile_x == 20);
        REQUIRE(p2.tile_y == 15);
        REQUIRE(p2.width_tiles == 2.0f);
        REQUIRE(p2.height_tiles == 0.8f);
        REQUIRE(p2.behavior_type == PlatformBehaviorType::Horizontal);
        REQUIRE(p2.features.size() == 1);
        REQUIRE(p2.features[0] == PlatformFeatureType::Spikes);
    }

    SECTION(
        "Third platform (oscillating with multiple features) imported "
        "correctly") {
        const auto& p3 = level.platforms[2];
        REQUIRE(p3.tile_x == 8);
        REQUIRE(p3.tile_y == 25);
        REQUIRE(p3.width_tiles == 4.0f);
        REQUIRE(p3.height_tiles == 1.0f);
        REQUIRE(p3.behavior_type == PlatformBehaviorType::OscillatingSize);
        REQUIRE(p3.features.size() == 2);

        // Check both features are present
        bool has_spikes =
            std::find(p3.features.begin(),
                      p3.features.end(),
                      PlatformFeatureType::Spikes) != p3.features.end();
        bool has_checkpoint =
            std::find(p3.features.begin(),
                      p3.features.end(),
                      PlatformFeatureType::Checkpoint) != p3.features.end();
        REQUIRE(has_spikes);
        REQUIRE(has_checkpoint);
    }

    // Clean up
    std::filesystem::remove(temp_path);
}

TEST_CASE("Platform Level JSON Round Trip", "[json][roundtrip][platform]") {
    Editor editor1, editor2;
    editor1.init();
    editor2.init();

    // Create original level
    Level& level1 = editor1.get_test_level();
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
    std::string temp_path =
        std::filesystem::temp_directory_path() / "test_roundtrip.json";
    editor1.test_export_platform_level_json(temp_path);

    // Import with second editor
    editor2.test_import_platform_level_json(temp_path);

    Level& level2 = editor2.get_test_level();

    SECTION("Player spawn preserved in round trip") {
        REQUIRE(level2.player_spawn_x == level1.player_spawn_x);
        REQUIRE(level2.player_spawn_y == level1.player_spawn_y);
    }

    SECTION("Platform data preserved in round trip") {
        REQUIRE(level2.platforms.size() == 1);
        const auto& imported_platform = level2.platforms[0];

        REQUIRE(imported_platform.tile_x == original_platform.tile_x);
        REQUIRE(imported_platform.tile_y == original_platform.tile_y);
        REQUIRE(imported_platform.width_tiles == original_platform.width_tiles);
        REQUIRE(imported_platform.height_tiles ==
                original_platform.height_tiles);
        REQUIRE(imported_platform.behavior_type ==
                original_platform.behavior_type);
        REQUIRE(imported_platform.features == original_platform.features);
    }

    // Clean up
    std::filesystem::remove(temp_path);
}

TEST_CASE("Invalid JSON Handling", "[json][error][platform]") {
    Editor editor;
    editor.init();

    SECTION("Missing file") {
        // Should not crash when importing non-existent file
        REQUIRE_NOTHROW(
            editor.test_import_platform_level_json("non_existent_file.json"));
    }

    SECTION("Invalid JSON syntax") {
        std::string invalid_json = "{ invalid json syntax }";
        std::string temp_path = create_temp_file(invalid_json);

        // Should not crash on invalid JSON
        REQUIRE_NOTHROW(editor.test_import_platform_level_json(temp_path));

        std::filesystem::remove(temp_path);
    }

    SECTION("Missing required fields") {
        nlohmann::json incomplete_json = {
            {"platforms",
             {{
                 {"x", 5}, {"y", 10}
                 // Missing width, height, behavior
             }}}};

        std::string temp_path = create_temp_file(incomplete_json.dump());

        // Should handle missing fields gracefully
        REQUIRE_NOTHROW(editor.test_import_platform_level_json(temp_path));

        std::filesystem::remove(temp_path);
    }

    SECTION("Unknown behavior type") {
        nlohmann::json unknown_behavior_json = {
            {"platforms",
             {{{"x", 5},
               {"y", 10},
               {"width", 2.0},
               {"height", 1.0},
               {"behavior", "unknown_behavior_type"}}}}};

        std::string temp_path = create_temp_file(unknown_behavior_json.dump());

        // Should handle unknown behavior gracefully (likely defaults to static)
        REQUIRE_NOTHROW(editor.test_import_platform_level_json(temp_path));

        Level& level = editor.get_test_level();
        if (!level.platforms.empty()) {
            // Should default to static behavior for unknown types
            REQUIRE(level.platforms[0].behavior_type ==
                    PlatformBehaviorType::Static);
        }

        std::filesystem::remove(temp_path);
    }
}
