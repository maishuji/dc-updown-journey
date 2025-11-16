// Copyright 2025 Quentin Cartier
#pragma once

#include <string>
#include <cstdio>  // FILE, fopen, fclose

namespace udj::core {

/**
 * @brief Core utilities for file and asset management
 */
namespace filesystem {

/**
 * @brief Constructs the full path for an asset file based on the platform's
 * base asset directory.
 *
 * The base asset directory is platform-dependent, and it is either `assets/`
 * for Linux or `romdisk/` for Dreamcast.
 *
 * @param filename The filename of the asset to retrieve the full path for.
 *                 This is expected to be a relative path (e.g., "image.png").
 *
 * @return A `std::string` containing the full path to the asset.
 */
inline std::string get_assets_path(const std::string& filename) {
    return std::string(ASSETS_BASE_PATH) + filename;
}

/**
 * @brief Check if a file exists on the filesystem
 *
 * @param filepath The path to the file to check
 * @return true if the file exists, false otherwise
 */
inline bool file_exists(const std::string& filepath) {
    FILE* file = fopen(filepath.c_str(), "r");
    if (file) {
        fclose(file);
        return true;
    }
    return false;
}

}  // namespace filesystem

/**
 * @brief Mathematical utility functions
 */
namespace math {

namespace internal {
inline constexpr float kEpsilonDefault = 0.001F;
}  // namespace internal

/**
 * @brief Check if a floating point value is near zero
 */
inline constexpr bool is_near_zero(float value,
                                   float epsilon = internal::kEpsilonDefault) {
    return (value > -epsilon && value < epsilon);
}

/**
 * @brief Check if two floating point values have the same sign
 */
inline constexpr bool is_same_sign(float valueA, float valueB) {
    return valueA * valueB >= 0.0F;
}

}  // namespace math

}  // namespace udj::core

// Backward compatibility aliases for existing code
namespace udjourney::coreutils {
// Filesystem functions
using udj::core::filesystem::get_assets_path;
using udj::core::filesystem::file_exists;

// Math utilities
namespace math = udj::core::math;
}  // namespace udjourney::coreutils
