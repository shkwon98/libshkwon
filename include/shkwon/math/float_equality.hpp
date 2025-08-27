#pragma once

#include <cmath>
#include <cstdlib>
#include <limits>
#include <type_traits>

namespace shkwon::math
{

/**
 * @brief Checks if two floating point values are approximately equal
 *
 * @tparam T Floating point type (float or double)
 * @param a First value to compare
 * @param b Second value to compare
 * @param epsilon Maximum relative difference allowed (defaults to machine epsilon for type T)
 * @return true if values are approximately equal within epsilon margin
 *
 * @details Uses the larger of the two values to determine the acceptable margin.
 * This is a 'weaker' check of equality than essentially-equal for any given epsilon.
 *
 * Example:
 * ApproximatelyEqual(95.1, 100.0, 0.05) => true
 *
 * With epsilon = 5%, 95.1 is approximately 100.0, as it falls within
 * the 5% margin of the largest value (100.0).
 */
template <typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
inline bool ApproximatelyEqual(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
{
    return std::fabs(a - b) <= ((std::fabs(a) < std::fabs(b) ? std::fabs(b) : std::fabs(a)) * epsilon);
}

/**
 * @brief Checks if two floating point values are essentially equal
 *
 * @tparam T Floating point type (float or double)
 * @param a First value to compare
 * @param b Second value to compare
 * @param epsilon Maximum relative difference allowed (defaults to machine epsilon for type T)
 * @return true if values are essentially equal within epsilon margin
 *
 * @details Uses the smaller of the two values to determine the acceptable margin.
 * This is a 'stronger' check of equality than approximately-equal for any given epsilon.
 *
 * Example:
 * EssentiallyEqual(95.1, 100.0, 0.05) => false
 *
 * 95.1 is NOT essentially 100.0, as 100.0 is not within a 5% difference
 * from 95.1 (the smallest value).
 */
template <typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
inline bool EssentiallyEqual(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
{
    return std::fabs(a - b) <= ((std::fabs(a) > std::fabs(b) ? std::fabs(b) : std::fabs(a)) * epsilon);
}

/**
 * @brief Checks if first value is definitely greater than second value
 *
 * @tparam T Floating point type (float or double)
 * @param a First value to compare
 * @param b Second value to compare
 * @param epsilon Maximum relative difference allowed (defaults to machine epsilon for type T)
 * @return true if a is definitely greater than b accounting for floating point errors
 *
 * @details Ensures the difference exceeds the margin of error determined by epsilon
 * and the larger of the two values.
 */
template <typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
inline bool DefinitelyGreaterThan(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
{
    return (a - b) > ((std::fabs(a) < std::fabs(b) ? std::fabs(b) : std::fabs(a)) * epsilon);
}

/**
 * @brief Checks if first value is definitely less than second value
 *
 * @tparam T Floating point type (float or double)
 * @param a First value to compare
 * @param b Second value to compare
 * @param epsilon Maximum relative difference allowed (defaults to machine epsilon for type T)
 * @return true if a is definitely less than b accounting for floating point errors
 *
 * @details Ensures the difference exceeds the margin of error determined by epsilon
 * and the larger of the two values.
 */
template <typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>>
inline bool DefinitelyLessThan(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
{
    return (b - a) > ((std::fabs(a) < std::fabs(b) ? std::fabs(b) : std::fabs(a)) * epsilon);
}

//=== Specific instantiations for backward compatibility ===//

inline bool ApproximatelyEqual(float a, float b, float epsilon = std::numeric_limits<float>::epsilon())
{
    return ApproximatelyEqual<float>(a, b, epsilon);
}

inline bool ApproximatelyEqual(double a, double b, double epsilon = std::numeric_limits<double>::epsilon())
{
    return ApproximatelyEqual<double>(a, b, epsilon);
}

inline bool EssentiallyEqual(float a, float b, float epsilon = std::numeric_limits<float>::epsilon())
{
    return EssentiallyEqual<float>(a, b, epsilon);
}

inline bool EssentiallyEqual(double a, double b, double epsilon = std::numeric_limits<double>::epsilon())
{
    return EssentiallyEqual<double>(a, b, epsilon);
}

inline bool DefinitelyGreaterThan(float a, float b, float epsilon = std::numeric_limits<float>::epsilon())
{
    return DefinitelyGreaterThan<float>(a, b, epsilon);
}

inline bool DefinitelyGreaterThan(double a, double b, double epsilon = std::numeric_limits<double>::epsilon())
{
    return DefinitelyGreaterThan<double>(a, b, epsilon);
}

inline bool DefinitelyLessThan(float a, float b, float epsilon = std::numeric_limits<float>::epsilon())
{
    return DefinitelyLessThan<float>(a, b, epsilon);
}

inline bool DefinitelyLessThan(double a, double b, double epsilon = std::numeric_limits<double>::epsilon())
{
    return DefinitelyLessThan<double>(a, b, epsilon);
}

} // namespace shkwon::math
