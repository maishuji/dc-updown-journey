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
    TilePanel panel;
    panel.set_button("Brick", color::kColorRed);
    EXPECT_EQ(panel.get_current_color(), color::kColorRed);
    panel.set_button("Grass", color::kColorGreen);
    EXPECT_EQ(panel.get_current_color(), color::kColorGreen);
}
