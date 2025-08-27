#pragma once

#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

namespace shkwon::unit
{

/**
 * @brief Class representing an angle in degrees
 *
 * Degrees represents an angular measurement between 0.0 and 360.0 degrees,
 * measured clockwise. Values outside this range can be normalized.
 * Negative values are interpreted as counter-clockwise and converted to
 * their clockwise equivalent.
 *
 * Note that for geographic coordinates:
 * - latitude ranges from -90 to 90 degrees
 * - longitude ranges from -180 to 180 degrees
 */
class Degrees
{
public:
    //=== Constructors ===//

    /** @brief Default constructor, initializes to 0 degrees */
    constexpr Degrees() = default;

    /**
     * @brief Constructor that takes a double value representing degrees
     * @param[in] init Initial angle in degrees
     */
    constexpr Degrees(double init)
        : value{ init }
    {
    }

    //=== Conversion Methods ===//

    /**
     * @brief Conversion operator to double
     * @return The angle in degrees as a double
     */
    constexpr operator double() const
    {
        return value;
    }

    /**
     * @brief Convert to double value
     * @return The angle in degrees as a double
     */
    constexpr double ToDouble() const
    {
        return value;
    }

    /**
     * @brief Convert to float value
     * @return The angle in degrees as a float
     */
    constexpr float ToFloat() const
    {
        return static_cast<float>(value);
    }

    /**
     * @brief Convert to unsigned integer
     * @return The angle in degrees rounded to the nearest unsigned integer
     */
    constexpr unsigned int ToUint() const
    {
        return static_cast<unsigned int>(std::round(value));
    }

    /**
     * @brief Convert to radians
     * @return The angle converted to radians
     */
    constexpr double ToRadians() const
    {
        return value * (M_PI / 180.0);
    }

    /**
     * @brief Convert to string with degree symbol
     * @return String representation of the angle with degree symbol
     */
    std::string ToString() const
    {
        std::ostringstream os{};
        os << std::fixed << value << "°";
        return os.str();
    }

    /**
     * @brief Stream insertion operator
     * @param os Output stream
     * @param a Degrees object to output
     * @return Reference to the output stream
     */
    friend std::ostream &operator<<(std::ostream &os, const Degrees &a)
    {
        os << a.ToString();
        return os;
    }

    //=== Comparison Operators ===//

    /**
     * @brief Equality comparison operator
     * @param rhs Right-hand side Degrees object
     * @return True if angles are equal, accounting for circular wrapping
     *
     * Degrees are considered 'equal' if the difference is less than epsilon,
     * taking into account that angles wrap at 360 degrees.
     */
    constexpr bool operator==(const Degrees &rhs) const
    {
        // Degrees are considered 'equal' if the difference is less
        // than could be measured by the encoder.
        // Since we are operating on a circle and angle values wrap
        // at 360 degrees, the delta between the left-hand-side and
        // right-hand-side values could be either approaching zero,
        // or approaching 360 degrees.
        //
        const auto &lhs{ *this };
        constexpr double epsilon = std::numeric_limits<double>::epsilon();
        bool approaching_0{ std::abs(lhs.value - rhs.value) <= epsilon };
        bool approaching_360{ (360.0 - std::abs(lhs.value - rhs.value)) <= epsilon };

        return approaching_0 || approaching_360;
    }

    /**
     * @brief Inequality comparison operator
     * @param rhs Right-hand side Degrees object
     * @return True if angles are not equal
     */
    constexpr bool operator!=(const Degrees &rhs) const
    {
        return !operator==(rhs);
    }

    /**
     * @brief Less-than comparison operator
     * @param rhs Right-hand side Degrees object
     * @return True if this angle is less than rhs
     */
    constexpr bool operator<(const Degrees &rhs) const
    {
        return (this->value < rhs.value);
    }

    /**
     * @brief Greater-than comparison operator
     * @param rhs Right-hand side Degrees object
     * @return True if this angle is greater than rhs
     */
    constexpr bool operator>(const Degrees &rhs) const
    {
        return (this->value > rhs.value);
    }

    /**
     * @brief Less-than-or-equal comparison operator
     * @param rhs Right-hand side Degrees object
     * @return True if this angle is less than or equal to rhs
     */
    constexpr bool operator<=(const Degrees &rhs) const
    {
        return ((*this == rhs) || (*this < rhs));
    }

    /**
     * @brief Greater-than-or-equal comparison operator
     * @param rhs Right-hand side Degrees object
     * @return True if this angle is greater than or equal to rhs
     */
    constexpr bool operator>=(const Degrees &rhs) const
    {
        return ((*this == rhs) || (*this > rhs));
    }

    //=== Addition/Subtraction Operators ===//

    /**
     * @brief Addition operator
     * @param rhs Right-hand side Degrees object
     * @return New Degrees object with the sum of the angles
     */
    constexpr Degrees operator+(const Degrees &rhs) const
    {
        return Degrees{ this->value + rhs.value };
    }

    /**
     * @brief Subtraction operator
     * @param rhs Right-hand side Degrees object
     * @return New Degrees object with the difference of the angles
     */
    constexpr Degrees operator-(const Degrees &rhs) const
    {
        return Degrees{ this->value - rhs.value };
    }

    /**
     * @brief Addition assignment operator
     * @param rhs Right-hand side Degrees object
     * @return Reference to this object after addition
     */
    constexpr Degrees &operator+=(const Degrees &rhs)
    {
        value += rhs.value;
        return *this;
    }

    /**
     * @brief Subtraction assignment operator
     * @param rhs Right-hand side Degrees object
     * @return Reference to this object after subtraction
     */
    constexpr Degrees &operator-=(const Degrees &rhs)
    {
        value -= rhs.value;
        return *this;
    }

    //=== Arithmetic Operators ===//

    /**
     * @brief Unary plus operator
     * @return Copy of this object
     */
    constexpr Degrees operator+() const
    {
        return *this;
    }

    /**
     * @brief Unary minus operator
     * @return New Degrees object with negated angle
     */
    constexpr Degrees operator-() const
    {
        return Degrees{ -this->value };
    }

    //=== Multiplication/Division Operators ===//

    /**
     * @brief Multiplication by integer operator
     * @param rhs Integer multiplier
     * @return New Degrees object with the angle multiplied by rhs
     */
    constexpr Degrees operator*(const int &rhs) const
    {
        return Degrees{ this->value * rhs };
    }

    /**
     * @brief Multiplication by double operator
     * @param rhs Double multiplier
     * @return New Degrees object with the angle multiplied by rhs
     */
    constexpr Degrees operator*(const double &rhs) const
    {
        return Degrees{ this->value * rhs };
    }

    /**
     * @brief Division by integer operator
     * @param rhs Integer divisor
     * @return New Degrees object with the angle divided by rhs
     */
    constexpr Degrees operator/(const int &rhs) const
    {
        return Degrees{ this->value / rhs };
    }

    /**
     * @brief Division by double operator
     * @param rhs Double divisor
     * @return New Degrees object with the angle divided by rhs
     */
    constexpr Degrees operator/(const double &rhs) const
    {
        return Degrees{ this->value / rhs };
    }

    /**
     * @brief Multiplication assignment by integer operator
     * @param rhs Integer multiplier
     * @return Reference to this object after multiplication
     */
    constexpr Degrees &operator*=(const int &rhs)
    {
        value *= rhs;
        return *this;
    }

    /**
     * @brief Multiplication assignment by double operator
     * @param rhs Double multiplier
     * @return Reference to this object after multiplication
     */
    constexpr Degrees &operator*=(const double &rhs)
    {
        value *= rhs;
        return *this;
    }

    /**
     * @brief Division assignment by integer operator
     * @param rhs Integer divisor
     * @return Reference to this object after division
     */
    constexpr Degrees &operator/=(const int &rhs)
    {
        value /= rhs;
        return *this;
    }

    /**
     * @brief Division assignment by double operator
     * @param rhs Double divisor
     * @return Reference to this object after division
     */
    constexpr Degrees &operator/=(const double &rhs)
    {
        value /= rhs;
        return *this;
    }

    //=== Utility Methods ===//

    /**
     * @brief Normalize angle to a specific range
     * @param min Minimum value of the range (default: 0.0)
     * @param max Maximum value of the range (default: 360.0)
     * @return Reference to this object after normalization
     *
     * Adjusts the angle to be within the specified range [min, max)
     */
    Degrees &Normalize(double min = 0.0, double max = 360.0)
    {
        const double range = max - min;

        // Fast path for common case
        if (value >= min && value < max)
        {
            return *this;
        }

        // Normalize to [0, range)
        value = std::fmod(value - min, range);
        if (value < 0.0)
        {
            value += range;
        }

        // Shift to [min, max)
        value += min;

        return *this;
    }

private:
    double value{ 0.0 }; ///< The angle value in degrees
};

} // namespace shkwon::unit

/**
 * @brief User-defined literal for creating Degrees objects from floating-point values
 * @param value Angle in degrees as a long double
 * @return Degrees object initialized with the specified value
 */
constexpr shkwon::unit::Degrees operator""_deg(long double value)
{
    return shkwon::unit::Degrees{ static_cast<double>(value) };
}

/**
 * @brief User-defined literal for creating Degrees objects from integer values
 * @param value Angle in degrees as an unsigned long long
 * @return Degrees object initialized with the specified value
 */
constexpr shkwon::unit::Degrees operator""_deg(unsigned long long int value)
{
    return shkwon::unit::Degrees{ static_cast<double>(value) };
}
