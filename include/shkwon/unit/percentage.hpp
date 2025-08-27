#pragma once

#include <algorithm>

namespace shkwon::unit
{

/**
 * @brief Class representing a percentage value between 0 and 100
 *
 * This class encapsulates a percentage value that is internally
 * stored as a normalized float between 0.0 and 1.0.
 */
class Percentage
{
public:
    /** @brief Default constructor, initializes to 0% */
    constexpr Percentage() = default;

    /**
     * @brief Constructor that takes a float value representing a percentage
     * @param[in] init Initial percentage value between 0 and 100
     *
     * The value is clamped to range [0.0, 100.0] and stored internally as [0.0, 1.0]
     */
    constexpr Percentage(float init)
        : value{ std::clamp(init, 0.0f, 100.0f) / 100.0f }
    {
    }

    /**
     * @brief Conversion operator to float
     * @return The percentage value as a float between 0.0 and 1.0
     */
    constexpr operator float() const
    {
        return value;
    }

private:
    float value{ 0.0f }; ///< The percentage value stored as a normalized float between 0.0 and 1.0
};

} // namespace shkwon::unit

/**
 * @brief User-defined literal for creating Percentage objects from floating-point values
 * @param val Percentage value as a long double
 * @return Percentage object initialized with the specified value
 */
constexpr shkwon::unit::Percentage operator""_percent(long double val)
{
    return shkwon::unit::Percentage{ static_cast<float>(val) };
}

/**
 * @brief User-defined literal for creating Percentage objects from integer values
 * @param val Percentage value as an unsigned long long
 * @return Percentage object initialized with the specified value
 */
constexpr shkwon::unit::Percentage operator""_percent(unsigned long long val)
{
    return shkwon::unit::Percentage{ static_cast<float>(val) };
}
