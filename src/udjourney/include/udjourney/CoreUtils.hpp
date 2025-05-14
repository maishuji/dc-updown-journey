// Copyright 2025 Quentin Cartier
#ifndef SRC_UDJOURNEY_INCLUDE_UDJOURNEY_COREUTILS_HPP_
#define SRC_UDJOURNEY_INCLUDE_UDJOURNEY_COREUTILS_HPP_

#include <string>

namespace internal {
inline constexpr float kEpsilonDefault = 0.001F;
}  // namespace internal

namespace udjourney::coreutils {

/**
 * @brief Constructs the full path for an asset file based on the platform's
 * base asset directory.
 *
 * This function concatenates the platform-specific base path for assets
 * (defined by `ASSETS_BASE_PATH`) with the provided filename, creating a full
 * path to the asset.
 *
 * The base asset directory is platform-dependent, and it is either `assets` for
 * Linux or `romdisk` for Dreamcast (or similar, depending on your
 * configuration).
 *
 * @param iFilename The filename of the asset to retrieve the full path for.
 *                  This is expected to be a relative path (e.g., "image.png").
 *
 * @return A `std::string` containing the full path to the asset, which combines
 * the platform's base asset directory and the provided filename.
 *
 * @note This function assumes that `ASSETS_BASE_PATH` is correctly defined
 * elsewhere in your code, based on the target platform.
 */
inline std::string get_assets_path(const std::string& iFilename) {
    return std::string(ASSETS_BASE_PATH) + iFilename;
}

namespace math {

inline constexpr bool is_near_zero(float iValue,
                                   float iEpsilon = internal::kEpsilonDefault) {
    return (iValue > -iEpsilon && iValue < iEpsilon);
}
inline constexpr bool is_same_sign(float iValueA, float iValueB) {
    return iValueA * iValueB >= 0.0F;
}

}  // namespace math
}  // namespace udjourney::coreutils

#endif  // SRC_UDJOURNEY_INCLUDE_UDJOURNEY_COREUTILS_HPP_
