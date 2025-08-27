#pragma once

#include <cstdint>

#include "shkwon/unit/degrees.hpp"
#include "shkwon/unit/percentage.hpp"
#include "shkwon/unit/radians.hpp"

/**
 * @namespace shkwon::unit
 * @brief Namespace containing unit types and conversions for various measurements
 *
 * This namespace provides type definitions and utility functions for working
 * with different units of measurement, such as angles, distances, and speeds.
 */
namespace shkwon::unit
{

/**
 * @typedef Azimuth
 * @brief Type for representing an azimuth angle as an unsigned 16-bit integer
 *
 * Represents horizontal angular direction, typically measured clockwise from north.
 */
using Azimuth = std::uint16_t;

/**
 * @typedef Azimuth_num
 * @brief Alias for Azimuth type, used for mathematical operations on azimuth values
 */
using Azimuth_num = Azimuth;

/**
 * @typedef Encoder_step
 * @brief Type for representing encoder steps as an unsigned 16-bit integer
 *
 * Used for hardware encoders that measure rotation in discrete steps.
 */
using Encoder_step = std::uint16_t;

/**
 * @typedef Meters
 * @brief Type for representing distance measurements in meters as floating-point values
 */
using Meters = float;

/**
 * @typedef Meters_per_sec
 * @brief Type for representing speed measurements in meters per second as floating-point values
 */
using Meters_per_sec = float;

} // namespace shkwon::unit

//=== User-defined literals for unit types ===//

/**
 * @brief User-defined literal for creating Azimuth values
 * @param val Value as an unsigned long long
 * @return Value converted to the Azimuth_num type
 *
 * Example: auto az = 180_azimuths;
 */
constexpr inline shkwon::unit::Azimuth_num operator""_azimuths(unsigned long long val)
{
    return static_cast<shkwon::unit::Azimuth_num>(val);
}

/**
 * @brief User-defined literal for creating Meters values from integers
 * @param val Value in meters as an unsigned long long
 * @return Value converted to the Meters type
 *
 * Example: auto distance = 100_m;
 */
constexpr inline shkwon::unit::Meters operator""_m(unsigned long long val)
{
    return static_cast<shkwon::unit::Meters>(val);
}

/**
 * @brief User-defined literal for creating Meters values from floating-point numbers
 * @param val Value in meters as a long double
 * @return Value converted to the Meters type
 *
 * Example: auto distance = 3.5_m;
 */
constexpr inline shkwon::unit::Meters operator""_m(long double val)
{
    return static_cast<shkwon::unit::Meters>(val);
}

/**
 * @brief User-defined literal for creating Meters_per_sec values from integers
 * @param val Value in meters per second as an unsigned long long
 * @return Value converted to the Meters_per_sec type
 *
 * Example: auto speed = 5_mps;
 */
constexpr inline shkwon::unit::Meters_per_sec operator""_mps(unsigned long long val)
{
    return static_cast<shkwon::unit::Meters_per_sec>(val);
}

/**
 * @brief User-defined literal for creating Meters_per_sec values from floating-point numbers
 * @param val Value in meters per second as a long double
 * @return Value converted to the Meters_per_sec type
 *
 * Example: auto speed = 1.5_mps;
 */
constexpr inline shkwon::unit::Meters_per_sec operator""_mps(long double val)
{
    return static_cast<shkwon::unit::Meters_per_sec>(val);
}
