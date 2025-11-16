// Copyright 2025 Quentin Cartier
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "udjourney-editor/Editor.hpp"
#include "udjourney-editor/Level.hpp"
#include "helpers/test_helpers.hpp"

class TilemapJSONTest : public ::testing::Test {
 protected:
    void SetUp() override { editor.init(); }

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

    Editor editor;
    std::vector<std::string> temp_files;
};

TEST_F(TilemapJSONTest, TilemapJSONExport) {
    Level& level = editor.get_test_level();

    // Set up test tilemap data using the actual structure
    level.row_cnt = 8;
    level.col_cnt = 10;
    level.resize(level.row_cnt, level.col_cnt);

    // Fill with test pattern using Cell objects with different colors
    const ImU32 WALL_COLOR = IM_COL32(128, 128, 128, 255);  // Gray for walls
    const ImU32 GROUND_COLOR = IM_COL32(139, 69, 19, 255);  // Brown for ground
    const ImU32 AIR_COLOR = IM_COL32(255, 255, 255, 255);   // White for air

    for (size_t row = 0; row < level.row_cnt; ++row) {
        for (size_t col = 0; col < level.col_cnt; ++col) {
            size_t index = row * level.col_cnt + col;
            Cell cell;
            if (row == 0 || row == level.row_cnt - 1 || col == 0 ||
                col == level.col_cnt - 1) {
                cell.color = WALL_COLOR;  // Border walls
            } else if (row == level.row_cnt - 2) {
                cell.color = GROUND_COLOR;  // Ground line
            } else {
                cell.color = AIR_COLOR;  // Empty space
            }
            level.tiles[index] = cell;
        }
    }

    // Set player spawn
    level.player_spawn_x = 2;
    level.player_spawn_y = 5;

    // Export to temporary file
    std::string temp_path = createTempFile("tilemap_export.json");
    editor.test_export_tilemap_json(temp_path);

    // Verify file was created
    ASSERT_TRUE(std::filesystem::exists(temp_path));

    // Parse and verify JSON content
    std::ifstream file(temp_path);
    nlohmann::json exported_json;
    file >> exported_json;

    // Level dimensions are exported correctly
    EXPECT_TRUE(exported_json.contains("rows"));
    EXPECT_TRUE(exported_json.contains("cols"));
    EXPECT_EQ(exported_json["rows"], 8);
    EXPECT_EQ(exported_json["cols"], 10);

    // Tilemap data is exported correctly
    EXPECT_TRUE(exported_json.contains("tiles"));
    EXPECT_TRUE(exported_json["tiles"].is_array());
    EXPECT_EQ(exported_json["tiles"].size(), 80);  // 8 * 10

    // Check the structure contains row, col, and color
    auto& first_tile = exported_json["tiles"][0];
    EXPECT_TRUE(first_tile.contains("row"));
    EXPECT_TRUE(first_tile.contains("col"));
    EXPECT_TRUE(first_tile.contains("color"));

    // Check specific tile colors
    bool found_wall = false, found_ground = false, found_air = false;
    for (const auto& tile : exported_json["tiles"]) {
        ImU32 color = tile["color"].get<ImU32>();
        if (color == WALL_COLOR) found_wall = true;
        if (color == GROUND_COLOR) found_ground = true;
        if (color == AIR_COLOR) found_air = true;
    }
    EXPECT_TRUE(found_wall);
    EXPECT_TRUE(found_ground);
    EXPECT_TRUE(found_air);
}

TEST_CASE("Tilemap JSON Import", "[json][import][tilemap]") {
    Editor editor;
    editor.init();

    // Create test JSON content using the actual tilemap format
    const ImU32 WALL_COLOR = IM_COL32(128, 128, 128, 255);
    const ImU32 AIR_COLOR = IM_COL32(255, 255, 255, 255);
    const ImU32 GROUND_COLOR = IM_COL32(139, 69, 19, 255);

    nlohmann::json test_json = {
        {"rows", 4},
        {"cols", 5},
        {"tiles",
         {{{"row", 0}, {"col", 0}, {"color", WALL_COLOR}},
          {{"row", 0}, {"col", 1}, {"color", WALL_COLOR}},
          {{"row", 0}, {"col", 2}, {"color", WALL_COLOR}},
          {{"row", 0}, {"col", 3}, {"color", WALL_COLOR}},
          {{"row", 0}, {"col", 4}, {"color", WALL_COLOR}},
          {{"row", 1}, {"col", 0}, {"color", WALL_COLOR}},
          {{"row", 1}, {"col", 1}, {"color", AIR_COLOR}},
          {{"row", 1}, {"col", 2}, {"color", AIR_COLOR}},
          {{"row", 1}, {"col", 3}, {"color", AIR_COLOR}},
          {{"row", 1}, {"col", 4}, {"color", WALL_COLOR}},
          {{"row", 2}, {"col", 0}, {"color", WALL_COLOR}},
          {{"row", 2}, {"col", 1}, {"color", AIR_COLOR}},
          {{"row", 2}, {"col", 2}, {"color", GROUND_COLOR}},
          {{"row", 2}, {"col", 3}, {"color", AIR_COLOR}},
          {{"row", 2}, {"col", 4}, {"color", WALL_COLOR}},
          {{"row", 3}, {"col", 0}, {"color", WALL_COLOR}},
          {{"row", 3}, {"col", 1}, {"color", WALL_COLOR}},
          {{"row", 3}, {"col", 2}, {"color", WALL_COLOR}},
          {{"row", 3}, {"col", 3}, {"color", WALL_COLOR}},
          {{"row", 3}, {"col", 4}, {"color", WALL_COLOR}}}}};

    // Write to temporary file
    std::string temp_path = create_temp_file(test_json.dump(2));

    // Import the file
    editor.test_import_tilemap_json(temp_path);

    Level& level = editor.get_test_level();

    SECTION("Level dimensions imported correctly") {
        REQUIRE(level.row_cnt == 4);
        REQUIRE(level.col_cnt == 5);
    }

    SECTION("Tilemap data imported correctly") {
        REQUIRE(level.tiles.size() == 20);  // 4 * 5

        // Check specific tiles by color
        REQUIRE(level.tiles[0].color == WALL_COLOR);  // Top-left corner
        REQUIRE(level.tiles[4].color == WALL_COLOR);  // Top-right corner
        REQUIRE(level.tiles[6].color == AIR_COLOR);   // Interior air
        REQUIRE(level.tiles[12].color ==
                GROUND_COLOR);                         // Ground tile in middle
        REQUIRE(level.tiles[19].color == WALL_COLOR);  // Bottom-right corner

        // Check borders are all walls
        for (size_t x = 0; x < 5; ++x) {
            REQUIRE(level.tiles[x].color == WALL_COLOR);       // Top row
            REQUIRE(level.tiles[15 + x].color == WALL_COLOR);  // Bottom row
        }
        for (size_t y = 0; y < 4; ++y) {
            REQUIRE(level.tiles[y * 5].color == WALL_COLOR);  // Left column
            REQUIRE(level.tiles[y * 5 + 4].color ==
                    WALL_COLOR);  // Right column
        }
    }

    // Clean up
    std::filesystem::remove(temp_path);
}

TEST_CASE("Tilemap JSON Round Trip", "[json][roundtrip][tilemap]") {
    Editor editor1, editor2;
    editor1.init();
    editor2.init();

    // Create original level with specific pattern
    Level& level1 = editor1.get_test_level();
    level1.row_cnt = 3;
    level1.col_cnt = 6;
    level1.player_spawn_x = 4;
    level1.player_spawn_y = 1;
    level1.resize(level1.row_cnt, level1.col_cnt);

    const ImU32 WALL_COLOR = IM_COL32(128, 128, 128, 255);
    const ImU32 AIR_COLOR = IM_COL32(255, 255, 255, 255);
    const ImU32 GROUND_COLOR = IM_COL32(139, 69, 19, 255);

    // Set up specific pattern
    std::vector<ImU32> pattern = {WALL_COLOR,
                                  WALL_COLOR,
                                  WALL_COLOR,
                                  WALL_COLOR,
                                  WALL_COLOR,
                                  WALL_COLOR,
                                  WALL_COLOR,
                                  AIR_COLOR,
                                  GROUND_COLOR,
                                  AIR_COLOR,
                                  AIR_COLOR,
                                  WALL_COLOR,
                                  WALL_COLOR,
                                  GROUND_COLOR,
                                  GROUND_COLOR,
                                  GROUND_COLOR,
                                  GROUND_COLOR,
                                  WALL_COLOR};

    for (size_t i = 0; i < pattern.size(); ++i) {
        level1.tiles[i].color = pattern[i];
    }

    // Export to file
    std::string temp_path =
        std::filesystem::temp_directory_path() / "test_tilemap_roundtrip.json";
    editor1.test_export_tilemap_json(temp_path);

    // Import with second editor
    editor2.test_import_tilemap_json(temp_path);

    Level& level2 = editor2.get_test_level();

    SECTION("All level data preserved in round trip") {
        REQUIRE(level2.row_cnt == level1.row_cnt);
        REQUIRE(level2.col_cnt == level1.col_cnt);
        REQUIRE(level2.tiles.size() == level1.tiles.size());

        for (size_t i = 0; i < level1.tiles.size(); ++i) {
            REQUIRE(level2.tiles[i].color == level1.tiles[i].color);
        }
    }

    // Clean up
    std::filesystem::remove(temp_path);
}

TEST_CASE("Tilemap Invalid JSON Handling", "[json][error][tilemap]") {
    Editor editor;
    editor.init();

    SECTION("Missing tilemap file") {
        // Should not crash when importing non-existent file
        REQUIRE_NOTHROW(
            editor.test_import_tilemap_json("non_existent_tilemap.json"));
    }

    SECTION("Invalid tilemap JSON syntax") {
        std::string invalid_json = "{ invalid tilemap json }";
        std::string temp_path = create_temp_file(invalid_json);

        // Should not crash on invalid JSON
        REQUIRE_NOTHROW(editor.test_import_tilemap_json(temp_path));

        std::filesystem::remove(temp_path);
    }

    SECTION("Missing required fields") {
        nlohmann::json incomplete_json = {
            {"rows", 3},
            // Missing cols and tiles
        };

        std::string temp_path = create_temp_file(incomplete_json.dump());

        // Should handle missing fields gracefully
        REQUIRE_NOTHROW(editor.test_import_tilemap_json(temp_path));

        std::filesystem::remove(temp_path);
    }

    SECTION("Invalid tile structure") {
        nlohmann::json invalid_tiles_json = {
            {"rows", 2},
            {"cols", 2},
            {"tiles",
             {
                 {{"row", 0}, {"col", 0}},  // Missing color
                 {{"color", 12345}},        // Missing row/col
                 "invalid_structure",       // Not an object
                 {{"row", 1},
                  {"col", 1},
                  {"color", "not_a_number"}}  // Invalid color type
             }}};

        std::string temp_path = create_temp_file(invalid_tiles_json.dump());

        // Should handle invalid tile structures gracefully
        REQUIRE_NOTHROW(editor.test_import_tilemap_json(temp_path));

        std::filesystem::remove(temp_path);
    }
}

TEST_CASE("Empty Level Export/Import", "[json][edge-case]") {
    Editor editor1, editor2;
    editor1.init();
    editor2.init();

    // Create minimal level
    Level& level1 = editor1.get_test_level();
    level1.row_cnt = 1;
    level1.col_cnt = 1;
    level1.player_spawn_x = 0;
    level1.player_spawn_y = 0;
    level1.resize(1, 1);
    level1.tiles[0].color = IM_COL32(255, 255, 255, 255);  // White air
    level1.platforms.clear();

    // Export tilemap
    std::string tilemap_path =
        std::filesystem::temp_directory_path() / "empty_tilemap.json";
    editor1.test_export_tilemap_json(tilemap_path);

    // Export platform level
    std::string platform_path =
        std::filesystem::temp_directory_path() / "empty_platforms.json";
    editor1.test_export_platform_level_json(platform_path);

    // Import both with second editor
    editor2.test_import_tilemap_json(tilemap_path);
    editor2.test_import_platform_level_json(platform_path);

    Level& level2 = editor2.get_test_level();

    SECTION("Empty level data preserved") {
        REQUIRE(level2.row_cnt == 1);
        REQUIRE(level2.col_cnt == 1);
        REQUIRE(level2.player_spawn_x == 0);
        REQUIRE(level2.player_spawn_y == 0);
        REQUIRE(level2.tiles.size() == 1);
        REQUIRE(level2.tiles[0].color == IM_COL32(255, 255, 255, 255));
        REQUIRE(level2.platforms.empty());
    }

    // Clean up
    std::filesystem::remove(tilemap_path);
    std::filesystem::remove(platform_path);
}
