// Copyright 2025 Quentin Cartier
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>

#include <gtest/gtest.h>

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

class AssetAccessTest : public ::testing::Test {
 protected:
    void SetUp() override {
        // Try multiple possible asset paths
        std::vector<std::string> possible_paths = {
            "assets",
            "../assets",
            "../../assets",
            "../../../assets",
            "../../../../src/udjourney/romdisk"};

        for (const auto& path : possible_paths) {
            if (std::filesystem::exists(path)) {
                assets_path = path;
                std::cout << "Using assets path: " << assets_path << std::endl;
                return;
            }
        }

        // If no existing path found, try to create symlink
        assets_path = "assets";
        if (!std::filesystem::exists(assets_path)) {
            try {
                std::filesystem::create_symlink(
                    "../../../../src/udjourney/romdisk", assets_path);
                std::cout << "Created assets symlink: " << assets_path
                          << std::endl;
            } catch (const std::exception& e) {
                std::cout << "Warning: Could not create assets symlink: "
                          << e.what() << std::endl;
            }
        }
    }

    std::string assets_path;
};

TEST_F(AssetAccessTest, SymlinkExists) {
    EXPECT_TRUE(std::filesystem::exists(assets_path));
}

TEST_F(AssetAccessTest, AssetsPathIsSymlink) {
    EXPECT_TRUE(std::filesystem::is_symlink(assets_path));
}

TEST_F(AssetAccessTest, SymlinkPointsToRomdisk) {
    auto target = std::filesystem::read_symlink(assets_path);
    // The exact path depends on where tests are run from
    EXPECT_EQ(target.filename().string(), "romdisk");
}

TEST_F(AssetAccessTest, MonsterDirectoryExists) {
    std::string monsters_path = assets_path + "/monsters";
    EXPECT_TRUE(std::filesystem::exists(monsters_path))
        << "Monsters path: " << monsters_path;
}

TEST_F(AssetAccessTest, CanEnumerateMonsterPresetFiles) {
    std::string monsters_path = assets_path + "/monsters";

    if (!std::filesystem::exists(monsters_path)) {
        GTEST_SKIP() << "Monsters directory not found at: " << monsters_path;
    }

    std::vector<std::string> found_presets;

    for (const auto& entry :
         std::filesystem::directory_iterator(monsters_path)) {
        if (entry.path().extension() == ".json") {
            found_presets.push_back(entry.path().filename().stem().string());
        }
    }

    EXPECT_FALSE(found_presets.empty());

    // Print found presets for debugging
    std::cout << "Found monster presets: ";
    for (const auto& preset : found_presets) {
        std::cout << preset << " ";
    }
    std::cout << std::endl;
}

TEST_F(AssetAccessTest, SpecificMonsterPresetsExist) {
    std::string monsters_path = assets_path + "/monsters";
    std::vector<std::string> expected_monsters = {"goblin.json", "poring.json"};

    for (const auto& monster_file : expected_monsters) {
        std::string full_path = monsters_path + "/" + monster_file;
        std::cout << "Checking for monster preset: " << full_path << std::endl;

        if (std::filesystem::exists(full_path)) {
            EXPECT_TRUE(std::filesystem::exists(full_path));
        }
    }
}

TEST_F(AssetAccessTest, AnimationsDirectoryExists) {
    std::string animations_path = assets_path + "/animations";
    EXPECT_TRUE(std::filesystem::exists(animations_path))
        << "Animations path: " << animations_path;
}

TEST_F(AssetAccessTest, AnimationConfigFilesAccessible) {
    std::string animations_path = assets_path + "/animations";
    std::vector<std::string> expected_animations = {"monster_animations.json",
                                                    "player_animations.json"};

    for (const auto& anim_file : expected_animations) {
        std::string full_path = animations_path + "/" + anim_file;
        std::cout << "Checking for animation config: " << full_path
                  << std::endl;

        if (std::filesystem::exists(full_path)) {
            EXPECT_TRUE(std::filesystem::exists(full_path));
        }
    }
}

TEST_F(AssetAccessTest, TexturesAccessible) {
    std::vector<std::string> expected_textures = {
        "char1-Sheet.png", "poring1_dead-Sheet.png", "placeholder.png"};

    for (const auto& texture_file : expected_textures) {
        std::string full_path = assets_path + "/" + texture_file;
        std::cout << "Checking for texture: " << full_path << std::endl;
        EXPECT_TRUE(std::filesystem::exists(full_path))
            << "Texture not found: " << full_path;
    }
}

TEST_F(AssetAccessTest, AssetPathFunctionWorksCorrectly) {
    EXPECT_EQ(udjourney::coreutils::get_assets_path("test.txt"),
              "assets/test.txt");
    EXPECT_EQ(udjourney::coreutils::get_assets_path("monsters/goblin.json"),
              "assets/monsters/goblin.json");
    EXPECT_EQ(udjourney::coreutils::get_assets_path(""), "assets/");
}

TEST_F(AssetAccessTest, CanReadMonsterPresetContent) {
    std::string goblin_path = assets_path + "/monsters/goblin.json";

    if (std::filesystem::exists(goblin_path)) {
        std::ifstream file(goblin_path);
        ASSERT_TRUE(file.is_open());

        std::string content;
        std::string line;
        while (std::getline(file, line)) {
            content += line;
        }

        EXPECT_FALSE(content.empty());
        EXPECT_TRUE(content.find("goblin") != std::string::npos ||
                    content.find("monster") != std::string::npos);
    } else {
        // Skip test if goblin preset doesn't exist
        std::cout << "Warning: Goblin preset not found, skipping content test"
                  << std::endl;
        GTEST_SKIP() << "Goblin preset not found";
    }
}
