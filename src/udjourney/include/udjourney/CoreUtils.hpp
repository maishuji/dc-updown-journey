// Copyright 2025 Quentin Cartier
#ifndef SRC_UDJOURNEY_INCLUDE_UDJOURNEY_COREUTILS_HPP_
#define SRC_UDJOURNEY_INCLUDE_UDJOURNEY_COREUTILS_HPP_

namespace internal {
inline constexpr float kEpsilonDefault = 0.001F;
}  // namespace internal

namespace udjourney::coreutils::math {

inline constexpr bool is_near_zero(float value,
                                   float epsilon = internal::kEpsilonDefault) {
    return (value > -epsilon && value < epsilon);
}
inline constexpr bool is_same_sign(float iValueA, float iValueB) {
    return iValueA * iValueB >= 0.0F;
}

}  // namespace udjourney::coreutils::math

#endif  // SRC_UDJOURNEY_INCLUDE_UDJOURNEY_COREUTILS_HPP_
