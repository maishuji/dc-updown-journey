// Copyright 2025 Quentin Cartier
#include "udjourney/LevelMetadata.hpp"

#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>
#include <udj-core/Logger.hpp>

namespace fs = std::filesystem;

namespace udjourney {

std::vector<LevelMetadata> LevelMetadata::load_all_levels() {
    std::vector<LevelMetadata> levels;

    Logger::info("load_all_levels: Starting...");

    // Scan levels directory for JSON files
    std::string levels_dir = ASSETS_BASE_PATH "levels/";

    Logger::info("load_all_levels: Checking directory: %", levels_dir);

    try {
        if (!fs::exists(levels_dir)) {
            Logger::error("Levels directory not found: %", levels_dir);
            return levels;
        }

        Logger::info("load_all_levels: Directory exists, iterating...");

        for (const auto& entry : fs::directory_iterator(levels_dir)) {
            if (entry.path().extension() == ".json") {
                std::string filename = entry.path().filename().string();

                Logger::info("load_all_levels: Found JSON file: %", filename);

                // Skip UI screens (they're not playable levels)
                if (filename.find("screen") != std::string::npos ||
                    filename.find("title") != std::string::npos ||
                    filename.find("game_over") != std::string::npos ||
                    filename.find("win") != std::string::npos) {
                    Logger::info("load_all_levels: Skipping UI screen: %",
                                 filename);
                    continue;
                }

                Logger::info("load_all_levels: Loading metadata for: %",
                             filename);
                LevelMetadata metadata = load_from_file(filename);
                if (!metadata.id.empty()) {
                    levels.push_back(metadata);
                }
            }
        }

        Logger::info("Loaded % level(s) from directory", levels.size());
    } catch (const std::exception& e) {
        Logger::error("Error scanning levels directory: %", e.what());
    }

    return levels;
}

LevelMetadata LevelMetadata::load_from_file(const std::string& filename) {
    LevelMetadata metadata;

    try {
        std::string filepath = ASSETS_BASE_PATH "levels/" + filename;
        std::ifstream file(filepath);

        if (!file.is_open()) {
            Logger::warning("Could not open level file: %", filepath);
            return metadata;  // Empty
        }

        nlohmann::json json;
        file >> json;

        // Extract level ID from filename (e.g., "level1.json" -> "level1")
        metadata.filename = filename;
        metadata.id = filename.substr(0, filename.find_last_of('.'));

        // Load metadata from JSON (if present)
        if (json.contains("metadata")) {
            auto meta = json["metadata"];

            metadata.display_name = meta.value("display_name", metadata.id);
            metadata.difficulty = meta.value("difficulty", 1);
            metadata.thumbnail = meta.value("thumbnail", "");
        } else {
            // Default values - use level name as display name
            metadata.display_name = json.value("name", metadata.id);
            metadata.difficulty = 1;
        }

        // Load save data (unlocked, completed, best_time)
        metadata.unlocked = true;
        metadata.completed = false;
        metadata.best_time = 0;

        Logger::debug(
            "Loaded level metadata: % (%)", metadata.display_name, metadata.id);
    } catch (const std::exception& e) {
        Logger::error(
            "Error loading level metadata from %: %", filename, e.what());
    }
    return metadata;
}

}  // namespace udjourney
