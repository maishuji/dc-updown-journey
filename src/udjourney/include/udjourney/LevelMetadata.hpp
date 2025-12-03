// Copyright 2025 Quentin Cartier
#pragma once
#include <string>
#include <vector>

namespace udjourney {

/**
 * @brief Metadata for a game level
 *
 * Contains information about a level including display name, difficulty,
 * unlock status, and completion data.
 */
struct LevelMetadata {
    std::string id;            // e.g., "level1"
    std::string display_name;  // e.g., "Forest Escape"
    std::string filename;      // e.g., "level1.json"
    std::string thumbnail;     // Optional thumbnail path
    int difficulty;            // 1-5 stars
    bool unlocked;             // Is level unlocked?
    bool completed;            // Has player completed it?
    int best_time;  // Best completion time in seconds (0 = not completed)

    /**
     * @brief Load all available levels from the levels directory
     * @return Vector of level metadata
     */
    static std::vector<LevelMetadata> load_all_levels();

    /**
     * @brief Load level metadata from a specific file
     * @param filename Name of the level file (e.g., "level1.json")
     * @return Level metadata, or empty metadata if loading fails
     */
    static LevelMetadata load_from_file(const std::string& filename);
};

}  // namespace udjourney
