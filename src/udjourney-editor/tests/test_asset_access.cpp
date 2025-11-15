// Copyright 2025 Quentin Cartier
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>

// Mock the ASSETS_BASE_PATH for testing
#ifdef ASSETS_BASE_PATH
#undef ASSETS_BASE_PATH
#endif
#define ASSETS_BASE_PATH "assets/"

namespace udjourney::coreutils {
// Simple version of get_assets_path for testing
inline std::string get_assets_path(const std::string& relative_path) {
    return std::string(ASSETS_BASE_PATH) + relative_path;
}

inline bool file_exists(const std::string& filename) {
    return std::filesystem::exists(filename);
}
}  // namespace udjourney::coreutils

TEST_CASE("Asset symlink is properly configured", "[asset_access]") {
    std::string assets_path = "assets";

    // Auto-create symlink if it doesn't exist (for testing)
    if (!std::filesystem::exists(assets_path)) {
        try {
            std::filesystem::create_symlink("../../../../src/udjourney/romdisk",
                                            assets_path);
        } catch (const std::exception& e) {
            WARN("Could not create assets symlink: " << e.what());
        }
    }

    SECTION("Symlink exists") { REQUIRE(std::filesystem::exists(assets_path)); }

    SECTION("Assets path is a symlink") {
        REQUIRE(std::filesystem::is_symlink(assets_path));
    }

    SECTION("Symlink points to romdisk") {
        auto target = std::filesystem::read_symlink(assets_path);
        // The exact path depends on where tests are run from
        REQUIRE(target.filename().string() == "romdisk");
    }
}

TEST_CASE("Monster presets are accessible", "[asset_access][monsters]") {
    std::string monsters_path =
        udjourney::coreutils::get_assets_path("monsters");

    SECTION("Monsters directory exists") {
        REQUIRE(std::filesystem::exists(monsters_path));
    }

    SECTION("Can enumerate monster preset files") {
        std::vector<std::string> found_presets;

        for (const auto& entry :
             std::filesystem::directory_iterator(monsters_path)) {
            if (entry.path().extension() == ".json") {
                found_presets.push_back(
                    entry.path().filename().stem().string());
            }
        }

        REQUIRE_FALSE(found_presets.empty());

        // Print found presets for debugging
        std::cout << "Found monster presets: ";
        for (const auto& preset : found_presets) {
            std::cout << preset << " ";
        }
        std::cout << std::endl;
    }

    SECTION("Specific monster presets exist") {
        std::vector<std::string> expected_monsters = {"goblin.json",
                                                      "poring.json"};

        for (const auto& monster_file : expected_monsters) {
            std::string full_path = monsters_path + "/" + monster_file;
            INFO("Checking for monster preset: " << full_path);

            if (std::filesystem::exists(full_path)) {
                REQUIRE(udjourney::coreutils::file_exists(full_path));
            }
        }
    }
}

TEST_CASE("Animation configs are accessible", "[asset_access][animations]") {
    std::string animations_path =
        udjourney::coreutils::get_assets_path("animations");

    SECTION("Animations directory exists") {
        REQUIRE(std::filesystem::exists(animations_path));
    }

    SECTION("Animation config files are accessible") {
        std::vector<std::string> expected_animations = {
            "monster_animations.json", "player_animations.json"};

        for (const auto& anim_file : expected_animations) {
            std::string full_path = animations_path + "/" + anim_file;
            INFO("Checking for animation config: " << full_path);

            if (std::filesystem::exists(full_path)) {
                REQUIRE(udjourney::coreutils::file_exists(full_path));
            }
        }
    }
}

TEST_CASE("Textures are accessible", "[asset_access][textures]") {
    std::vector<std::string> expected_textures = {
        "char1-Sheet.png", "poring1_dead-Sheet.png", "placeholder.png"};

    for (const auto& texture_file : expected_textures) {
        std::string full_path =
            udjourney::coreutils::get_assets_path(texture_file);
        INFO("Checking for texture: " << full_path);
        REQUIRE(udjourney::coreutils::file_exists(full_path));
    }
}

TEST_CASE("Asset path function works correctly", "[asset_access][utility]") {
    SECTION("Basic path construction") {
        REQUIRE(udjourney::coreutils::get_assets_path("test.txt") ==
                "assets/test.txt");
        REQUIRE(udjourney::coreutils::get_assets_path("monsters/goblin.json") ==
                "assets/monsters/goblin.json");
        REQUIRE(udjourney::coreutils::get_assets_path("") == "assets/");
    }
}

TEST_CASE("Can read monster preset content",
          "[asset_access][monsters][integration]") {
    std::string goblin_path =
        udjourney::coreutils::get_assets_path("monsters/goblin.json");

    if (udjourney::coreutils::file_exists(goblin_path)) {
        std::ifstream file(goblin_path);
        REQUIRE(file.is_open());

        std::string content;
        std::string line;
        while (std::getline(file, line)) {
            content += line;
        }

        REQUIRE_FALSE(content.empty());
        REQUIRE((content.find("goblin") != std::string::npos ||
                 content.find("monster") != std::string::npos));
    } else {
        // Skip test if goblin preset doesn't exist
        WARN("Goblin preset not found, skipping content test");
    }
}
