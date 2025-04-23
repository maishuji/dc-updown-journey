// Copyright 2025 Quentin Cartier
#ifndef SRC_UDJOURNEY_INCLUDE_UDJOURNEY_COREUTILS_HPP_
#define SRC_UDJOURNEY_INCLUDE_UDJOURNEY_COREUTILS_HPP_

namespace udjourney::coreutils {
namespace math {
inline constexpr bool is_near_zero(float value, float epsilon = 0.001f) {
    return (value > -epsilon && value < epsilon);
}
inline constexpr bool is_same_sign(float a, float b) { return a * b >= 0.0f; }
}  // namespace math
}  // namespace udjourney::coreutils

#endif  // SRC_UDJOURNEY_INCLUDE_UDJOURNEY_COREUTILS_HPP_
