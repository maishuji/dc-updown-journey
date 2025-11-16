// Copyright 2025 Quentin Cartier
#include <filesystem>
#include <fstream>
#include <catch2/catch_test_macros.hpp>


#include "udjourney-editor/MonsterPresetManager.hpp"

TEST_CASE("MonsterPresetManager loads available presets",
          "[MonsterPresetManager]") {
    using namespace udjourney::editor;  // NOLINT(build/namespaces)

    SECTION("Manager loads presets from assets/monsters") {
        MonsterPresetManager manager;

        // Check if any presets were loaded
        INFO("Checking if monster presets were loaded from assets/monsters");
        CHECK(manager.has_presets());

        // Get preset names
        auto preset_names = manager.get_preset_names();
        INFO("Found " << preset_names.size() << " monster presets");

        // We know from previous tests that we should have at least these
        // presets
        bool found_goblin = false;
        bool found_spider = false;
        bool found_poring = false;

        for (const auto& name : preset_names) {
            INFO("Found preset: " << name);
            if (name == "goblin") found_goblin = true;
            if (name == "spider") found_spider = true;
            if (name == "poring") found_poring = true;
        }

        // At least one of our expected presets should be found
        CHECK((found_goblin || found_spider || found_poring));
    }

    SECTION("Manager provides detailed preset information") {
        MonsterPresetManager manager;

        if (manager.has_presets()) {
            auto preset_names = manager.get_preset_names();
            REQUIRE(!preset_names.empty());

            // Get the first preset and check its information
            const std::string& first_preset_name = preset_names[0];
            const auto* preset_info = manager.get_preset(first_preset_name);

            REQUIRE(preset_info != nullptr);
            CHECK(preset_info->is_valid);
            CHECK(!preset_info->name.empty());
            CHECK(!preset_info->display_name.empty());
            CHECK_GT!(preset_info->health, 0);
            CHECK_GT!(preset_info->speed, 0);
        }
    }

    SECTION("Manager handles missing presets gracefully") {
        MonsterPresetManager manager;

        // Try to get a preset that doesn't exist
        const auto* non_existent = manager.get_preset("non_existent_monster");
        CHECK(non_existent == nullptr);
    }
}
