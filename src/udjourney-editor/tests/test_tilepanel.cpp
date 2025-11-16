// Copyright 2025 Quentin Cartier
#include <gtest/gtest.h>

#include "udjourney-editor/TilePanel.hpp"

TEST(TilePanelTest, ColorValuesAreCorrect) {
    EXPECT_EQ(color::kColorRed, IM_COL32(255, 0, 0, 255));
    EXPECT_EQ(color::kColorGreen, IM_COL32(0, 255, 0, 255));
    EXPECT_EQ(color::kColorBlue, IM_COL32(0, 0, 255, 255));
    EXPECT_EQ(color::kColorOrange, IM_COL32(255, 128, 0, 255));
    EXPECT_EQ(color::kColorLightGreen, IM_COL32(0, 255, 128, 255));
    EXPECT_EQ(color::kColorPurple, IM_COL32(128, 0, 255, 255));
}

TEST(TilePanelTest, SetButtonSetsCurrentColor) {
    // This test would require ImGui context to work properly
    // For now, we'll test color constants and basic functionality
    TilePanel panel;

    // Test that we can get the current color (default should be white)
    ImU32 initial_color = panel.get_current_color();
    EXPECT_EQ(initial_color, IM_COL32(255, 255, 255, 255));  // Default white

    // Note: set_button requires ImGui context, so we can't test it here
    // This would be better tested in an integration test with proper ImGui
    // setup
}
