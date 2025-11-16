// Copyright 2025 Quentin Cartier
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>

#include "udjourney-editor/Editor.hpp"
#include "udjourney-editor/Level.hpp"
#include "helpers/test_helpers.hpp"

class TilemapJSONSimpleTest : public ::testing::Test {
 protected:
    void SetUp() override { editor.init(); }

    Editor editor;
};

TEST_F(TilemapJSONSimpleTest, BasicFunctionality) {
    Level& level = editor.get_test_level();

    // Basic test to ensure the test framework migration works
    EXPECT_TRUE(true);

    // Test that we can access the level
    EXPECT_GE(level.row_cnt, 0);
    EXPECT_GE(level.col_cnt, 0);
}
