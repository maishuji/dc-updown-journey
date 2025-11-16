// Copyright 2025 Quentin Cartier
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>

#include "udjourney-editor/MonsterPresetManager.hpp"

using namespace udjourney::editor;  // NOLINT(build/namespaces)

class MonsterPresetManagerTest : public ::testing::Test {
 protected:
    void SetUp() override {
        // Any setup needed for tests
    }
};

TEST_F(MonsterPresetManagerTest, LoadsAvailablePresets) {
    // Check if monsters directory exists first
    std::vector<std::string> possible_paths = {
        "assets/monsters", "../assets/monsters", "../../assets/monsters"};
    bool monsters_dir_exists = false;

    for (const auto& path : possible_paths) {
        if (std::filesystem::exists(path)) {
            monsters_dir_exists = true;
            std::cout << "Found monsters directory at: " << path << std::endl;
            break;
        }
    }

    if (!monsters_dir_exists) {
        GTEST_SKIP()
            << "No monsters directory found, skipping preset loading test";
    }

    MonsterPresetManager manager;

    // Get preset names
    auto preset_names = manager.get_preset_names();
    std::cout << "Found " << preset_names.size() << " monster presets"
              << std::endl;

    if (manager.has_presets()) {
        // We know from previous tests that we should have at least these
        // presets
        bool found_goblin = false;
        bool found_spider = false;
        bool found_poring = false;

        for (const auto& name : preset_names) {
            std::cout << "Found preset: " << name << std::endl;
            if (name == "goblin") found_goblin = true;
            if (name == "spider") found_spider = true;
            if (name == "poring") found_poring = true;
        }

        // At least one of our expected presets should be found
        EXPECT_TRUE(found_goblin || found_spider || found_poring)
            << "Should find at least one of: goblin, spider, or poring";
    } else {
        GTEST_SKIP() << "MonsterPresetManager could not load presets from "
                        "assets directory";
    }
}

TEST_F(MonsterPresetManagerTest, ProvidesDetailedPresetInformation) {
    MonsterPresetManager manager;

    if (manager.has_presets()) {
        auto preset_names = manager.get_preset_names();
        ASSERT_FALSE(preset_names.empty());

        // Get the first preset and check its information
        const std::string& first_preset_name = preset_names[0];
        const auto* preset_info = manager.get_preset(first_preset_name);

        ASSERT_NE(preset_info, nullptr);
        EXPECT_TRUE(preset_info->is_valid);
        EXPECT_FALSE(preset_info->name.empty());
        EXPECT_FALSE(preset_info->display_name.empty());
        EXPECT_GT(preset_info->health, 0);
        EXPECT_GT(preset_info->speed, 0);
    }
}

TEST_F(MonsterPresetManagerTest, HandlesMissingPresetsGracefully) {
    MonsterPresetManager manager;

    // Try to get a preset that doesn't exist
    const auto* non_existent = manager.get_preset("non_existent_monster");
    EXPECT_EQ(non_existent, nullptr);
}
