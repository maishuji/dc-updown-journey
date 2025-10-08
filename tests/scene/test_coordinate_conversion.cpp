// Copyright 2025 Quentin Cartier

#include <gtest/gtest.h>
#include <raylib/raylib.h>

#include "udjourney/scene/Scene.hpp"

using namespace udjourney::scene;

class CoordinateConversionTest : public ::testing::Test {
 protected:
    static constexpr float kExpectedTileSize = 32.0f;
    static constexpr float kFloatTolerance = 1e-6f;
};

// Test tile size constant
TEST_F(CoordinateConversionTest, TileSizeConstant) {
    EXPECT_FLOAT_EQ(Scene::kTileSize, kExpectedTileSize);
}

// Test basic tile to world position conversion
TEST_F(CoordinateConversionTest, TileToWorldPosition) {
    // Test origin
    Vector2 origin = Scene::tile_to_world_pos(0, 0);
    EXPECT_FLOAT_EQ(origin.x, 0.0f);
    EXPECT_FLOAT_EQ(origin.y, 0.0f);

    // Test positive coordinates
    Vector2 pos1 = Scene::tile_to_world_pos(1, 1);
    EXPECT_FLOAT_EQ(pos1.x, kExpectedTileSize);
    EXPECT_FLOAT_EQ(pos1.y, kExpectedTileSize);

    // Test larger coordinates
    Vector2 pos2 = Scene::tile_to_world_pos(5, 8);
    EXPECT_FLOAT_EQ(pos2.x, 5.0f * kExpectedTileSize);
    EXPECT_FLOAT_EQ(pos2.y, 8.0f * kExpectedTileSize);

    // Test with specific values
    Vector2 pos3 = Scene::tile_to_world_pos(10, 15);
    EXPECT_FLOAT_EQ(pos3.x, 320.0f);  // 10 * 32
    EXPECT_FLOAT_EQ(pos3.y, 480.0f);  // 15 * 32
}

// Test tile to world rectangle conversion
TEST_F(CoordinateConversionTest, TileToWorldRectangle) {
    // Test single tile rectangle
    Rectangle rect1 = Scene::tile_to_world_rect(0, 0, 1, 1);
    EXPECT_FLOAT_EQ(rect1.x, 0.0f);
    EXPECT_FLOAT_EQ(rect1.y, 0.0f);
    EXPECT_FLOAT_EQ(rect1.width, kExpectedTileSize);
    EXPECT_FLOAT_EQ(rect1.height, kExpectedTileSize);

    // Test multi-tile rectangle
    Rectangle rect2 = Scene::tile_to_world_rect(2, 3, 4, 5);
    EXPECT_FLOAT_EQ(rect2.x, 2.0f * kExpectedTileSize);       // 64
    EXPECT_FLOAT_EQ(rect2.y, 3.0f * kExpectedTileSize);       // 96
    EXPECT_FLOAT_EQ(rect2.width, 4.0f * kExpectedTileSize);   // 128
    EXPECT_FLOAT_EQ(rect2.height, 5.0f * kExpectedTileSize);  // 160

    // Test horizontal platform
    Rectangle rect3 = Scene::tile_to_world_rect(5, 10, 8, 1);
    EXPECT_FLOAT_EQ(rect3.x, 160.0f);      // 5 * 32
    EXPECT_FLOAT_EQ(rect3.y, 320.0f);      // 10 * 32
    EXPECT_FLOAT_EQ(rect3.width, 256.0f);  // 8 * 32
    EXPECT_FLOAT_EQ(rect3.height, 32.0f);  // 1 * 32

    // Test vertical platform
    Rectangle rect4 = Scene::tile_to_world_rect(12, 6, 2, 6);
    EXPECT_FLOAT_EQ(rect4.x, 384.0f);       // 12 * 32
    EXPECT_FLOAT_EQ(rect4.y, 192.0f);       // 6 * 32
    EXPECT_FLOAT_EQ(rect4.width, 64.0f);    // 2 * 32
    EXPECT_FLOAT_EQ(rect4.height, 192.0f);  // 6 * 32
}

// Test edge cases and boundary conditions
TEST_F(CoordinateConversionTest, EdgeCases) {
    // Test zero-width/height rectangles
    Rectangle zero_width = Scene::tile_to_world_rect(5, 5, 0, 3);
    EXPECT_FLOAT_EQ(zero_width.width, 0.0f);
    EXPECT_FLOAT_EQ(zero_width.height, 3.0f * kExpectedTileSize);

    Rectangle zero_height = Scene::tile_to_world_rect(5, 5, 3, 0);
    EXPECT_FLOAT_EQ(zero_height.width, 3.0f * kExpectedTileSize);
    EXPECT_FLOAT_EQ(zero_height.height, 0.0f);

    // Test large coordinates
    Vector2 large_pos = Scene::tile_to_world_pos(1000, 2000);
    EXPECT_FLOAT_EQ(large_pos.x, 32000.0f);
    EXPECT_FLOAT_EQ(large_pos.y, 64000.0f);
}

// Test coordinate consistency
TEST_F(CoordinateConversionTest, CoordinateConsistency) {
    int tile_x = 7;
    int tile_y = 11;
    int width = 3;
    int height = 2;

    // Get position and rectangle
    Vector2 pos = Scene::tile_to_world_pos(tile_x, tile_y);
    Rectangle rect = Scene::tile_to_world_rect(tile_x, tile_y, width, height);

    // Position should match rectangle corner
    EXPECT_FLOAT_EQ(pos.x, rect.x);
    EXPECT_FLOAT_EQ(pos.y, rect.y);

    // Rectangle dimensions should be correct
    EXPECT_FLOAT_EQ(rect.width, width * kExpectedTileSize);
    EXPECT_FLOAT_EQ(rect.height, height * kExpectedTileSize);
}

// Test mathematical relationships
TEST_F(CoordinateConversionTest, MathematicalRelationships) {
    // Test that scaling tiles scales world coordinates proportionally
    for (int scale = 1; scale <= 10; ++scale) {
        Vector2 pos1 = Scene::tile_to_world_pos(1, 1);
        Vector2 pos_scaled = Scene::tile_to_world_pos(scale, scale);

        EXPECT_FLOAT_EQ(pos_scaled.x, pos1.x * scale);
        EXPECT_FLOAT_EQ(pos_scaled.y, pos1.y * scale);
    }
}

// Test common game scenarios
TEST_F(CoordinateConversionTest, CommonGameScenarios) {
    // Standard platform sizes
    struct PlatformTest {
        int tile_x, tile_y, width, height;
        const char* description;
    };

    std::vector<PlatformTest> platform_tests = {
        {0, 15, 10, 1, "Ground platform"},
        {5, 12, 3, 1, "Small jumping platform"},
        {10, 8, 6, 2, "Large platform"},
        {20, 5, 1, 4, "Vertical wall"},
        {25, 0, 2, 20, "Tall barrier"}};

    for (const auto& test : platform_tests) {
        Rectangle rect = Scene::tile_to_world_rect(
            test.tile_x, test.tile_y, test.width, test.height);

        // Verify position
        EXPECT_FLOAT_EQ(rect.x, test.tile_x * kExpectedTileSize)
            << "Position X failed for: " << test.description;
        EXPECT_FLOAT_EQ(rect.y, test.tile_y * kExpectedTileSize)
            << "Position Y failed for: " << test.description;

        // Verify size
        EXPECT_FLOAT_EQ(rect.width, test.width * kExpectedTileSize)
            << "Width failed for: " << test.description;
        EXPECT_FLOAT_EQ(rect.height, test.height * kExpectedTileSize)
            << "Height failed for: " << test.description;

        // Verify non-negative values
        EXPECT_GE(rect.width, 0.0f)
            << "Negative width for: " << test.description;
        EXPECT_GE(rect.height, 0.0f)
            << "Negative height for: " << test.description;
    }
}

// Test player spawn scenarios
TEST_F(CoordinateConversionTest, PlayerSpawnScenarios) {
    struct SpawnTest {
        int tile_x, tile_y;
        float expected_world_x, expected_world_y;
        const char* description;
    };

    std::vector<SpawnTest> spawn_tests = {
        {0, 0, 0.0f, 0.0f, "Origin spawn"},
        {1, 10, 32.0f, 320.0f, "Typical spawn"},
        {5, 15, 160.0f, 480.0f, "Ground level spawn"},
        {10, 5, 320.0f, 160.0f, "Elevated spawn"}};

    for (const auto& test : spawn_tests) {
        Vector2 world_pos = Scene::tile_to_world_pos(test.tile_x, test.tile_y);

        EXPECT_FLOAT_EQ(world_pos.x, test.expected_world_x)
            << "World X failed for: " << test.description;
        EXPECT_FLOAT_EQ(world_pos.y, test.expected_world_y)
            << "World Y failed for: " << test.description;
    }
}

// Test fractional tile dimensions
TEST_F(CoordinateConversionTest, FractionalTileDimensions) {
    // Test 0.3 height platform (thin platform)
    Rectangle thin_platform = Scene::tile_to_world_rect(5, 10, 4.0f, 0.3f);
    EXPECT_FLOAT_EQ(thin_platform.x, 160.0f);      // 5 * 32
    EXPECT_FLOAT_EQ(thin_platform.y, 320.0f);      // 10 * 32
    EXPECT_FLOAT_EQ(thin_platform.width, 128.0f);  // 4 * 32
    EXPECT_FLOAT_EQ(thin_platform.height, 9.6f);   // 0.3 * 32
    
    // Test 0.5 width platform (half-width)
    Rectangle narrow_platform = Scene::tile_to_world_rect(2, 8, 0.5f, 2.0f);
    EXPECT_FLOAT_EQ(narrow_platform.x, 64.0f);      // 2 * 32
    EXPECT_FLOAT_EQ(narrow_platform.y, 256.0f);     // 8 * 32
    EXPECT_FLOAT_EQ(narrow_platform.width, 16.0f);  // 0.5 * 32
    EXPECT_FLOAT_EQ(narrow_platform.height, 64.0f); // 2 * 32
    
    // Test fractional dimensions for both width and height
    Rectangle small_platform = Scene::tile_to_world_rect(0, 0, 1.5f, 0.25f);
    EXPECT_FLOAT_EQ(small_platform.x, 0.0f);
    EXPECT_FLOAT_EQ(small_platform.y, 0.0f);
    EXPECT_FLOAT_EQ(small_platform.width, 48.0f);   // 1.5 * 32
    EXPECT_FLOAT_EQ(small_platform.height, 8.0f);   // 0.25 * 32
    
    // Test larger fractional values
    Rectangle large_fractional = Scene::tile_to_world_rect(3, 5, 2.75f, 1.8f);
    EXPECT_FLOAT_EQ(large_fractional.x, 96.0f);      // 3 * 32
    EXPECT_FLOAT_EQ(large_fractional.y, 160.0f);     // 5 * 32
    EXPECT_FLOAT_EQ(large_fractional.width, 88.0f);  // 2.75 * 32
    EXPECT_FLOAT_EQ(large_fractional.height, 57.6f); // 1.8 * 32
}

// Test win condition calculations for level design
TEST_F(CoordinateConversionTest, WinConditionCalculations) {
    // Test level1.json scenario: final platform at y=33, height=2
    // Level height should be (33 + 2) * 32 = 1120 pixels
    float final_platform_tile_y = 33.0f;
    float final_platform_height_tiles = 2.0f;
    float level_height_world = (final_platform_tile_y + final_platform_height_tiles) * kExpectedTileSize;
    EXPECT_FLOAT_EQ(level_height_world, 1120.0f);
    
    // Test win condition thresholds
    float win_threshold_98_percent = level_height_world * 0.98f;
    float win_threshold_90_percent = level_height_world * 0.90f;
    
    EXPECT_FLOAT_EQ(win_threshold_98_percent, 1097.6f);  // New improved threshold
    EXPECT_FLOAT_EQ(win_threshold_90_percent, 1008.0f);  // Old problematic threshold
    
    // Final platform starts at y=33*32=1056
    float final_platform_start = final_platform_tile_y * kExpectedTileSize;
    EXPECT_FLOAT_EQ(final_platform_start, 1056.0f);
    
    // Verify that 98% threshold requires player to be well onto the final platform
    EXPECT_GT(win_threshold_98_percent, final_platform_start);  // Must be past platform start
    EXPECT_LT(win_threshold_98_percent, level_height_world);    // But before very end
    
    // Verify that old 90% threshold was too early (before final platform)
    EXPECT_LT(win_threshold_90_percent, final_platform_start);  // This was the problem!
}
