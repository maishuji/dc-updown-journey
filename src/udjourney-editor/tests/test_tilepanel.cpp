#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include "udjourney-editor/TilePanel.hpp"

TEST_CASE("TilePanel color values are correct") {
    REQUIRE(color::kColorRed == IM_COL32(255, 0, 0, 255));
    REQUIRE(color::kColorGreen == IM_COL32(0, 255, 0, 255));
    REQUIRE(color::kColorBlue == IM_COL32(0, 0, 255, 255));
    REQUIRE(color::kColorOrange == IM_COL32(255, 128, 0, 255));
    REQUIRE(color::kColorLightGreen == IM_COL32(0, 255, 128, 255));
    REQUIRE(color::kColorPurple == IM_COL32(128, 0, 255, 255));
}

TEST_CASE("TilePanel set_button sets cur_color") {
    TilePanel panel;
    panel.set_button("Brick", color::kColorRed);
    REQUIRE(panel.get_current_color() == color::kColorRed);
    panel.set_button("Grass", color::kColorGreen);
    REQUIRE(panel.get_current_color() == color::kColorGreen);
}
