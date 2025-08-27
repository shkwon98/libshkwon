#pragma once

#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

namespace shkwon::unit
{

/**
 * @brief Class representing an angle in radians
 *
 * Radians represents an angular measurement between 0 and 2π radians,
 * measured clockwise. Values outside this range can be normalized.
 * Negative values are interpreted as counter-clockwise and converted to
 * their clockwise equivalent.
 */
class Radians
{
public:
    //=== Constructors ===//

    /** @brief Default constructor, initializes to 0 radians */
    constexpr Radians() = default;

    /**
     * @brief Constructor that takes a double value representing radians
     * @param[in] init Initial angle in radians
     */
    constexpr Radians(double init)
        : value{ init }
    {
    }

    //=== Conversion Methods ===//

    /**
     * @brief Conversion operator to double
     * @return The angle in radians as a double
     */
    constexpr operator double() const
    {
        return value;
    }

    /**
     * @brief Convert to double value
     * @return The angle in radians as a double
     */
    constexpr double ToDouble() const
    {
        return value;
    }

    /**
     * @brief Convert to float value
     * @return The angle in radians as a float
     */
    constexpr float ToFloat() const
    {
        return static_cast<float>(value);
    }

    /**
     * @brief Convert to unsigned integer
     * @return The angle in radians rounded to the nearest unsigned integer
     */
    constexpr unsigned int ToUint() const
    {
        return static_cast<unsigned int>(std::round(value));
    }

    /**
     * @brief Convert to degrees
     * @return The angle converted to degrees
     */
    constexpr double ToDegrees() const
    {
        return value * (180.0 / M_PI);
    }

    /**
     * @brief Convert to string with radians shkwon::unit
     * @return String representation of the angle with radians shkwon::unit
     */
    std::string ToString() const
    {
        std::ostringstream os{};
        os << std::fixed << value << "rad";
        return os.str();
    }

    /**
     * @brief Stream insertion operator
     * @param os Output stream
     * @param a Radians object to output
     * @return Reference to the output stream
     */
    friend std::ostream &operator<<(std::ostream &os, const Radians &a)
    {
        os << a.ToString();
        return os;
    }

    //=== Comparison Operators ===//

    /**
     * @brief Equality comparison operator
     * @param rhs Right-hand side Radians object
     * @return True if angles are equal, accounting for circular wrapping
     *
     * Radians are considered 'equal' if the difference is less than epsilon,
     * taking into account that angles wrap at 2π radians.
     */
    constexpr bool operator==(const Radians &rhs) const
    {
        // Radians are considered 'equal' if the difference is less
        // than could be measured by the encoder.
        // Since we are operating on a circle and angle values wrap
        // at 2π Radians, the delta between the left-hand-side and
        // right-hand-side values could be either approaching zero,
        // or approaching 2π Radians.
        //
        const auto &lhs{ *this };
        constexpr double epsilon = std::numeric_limits<double>::epsilon();
        bool approaching_0{ std::abs(lhs.value - rhs.value) <= epsilon };
        bool approaching_2pi{ ((2 * M_PI) - std::abs(lhs.value - rhs.value)) <= epsilon };

        return approaching_0 || approaching_2pi;
    }

    /**
     * @brief Inequality comparison operator
     * @param rhs Right-hand side Radians object
     * @return True if angles are not equal
     */
    constexpr bool operator!=(const Radians &rhs) const
    {
        return !operator==(rhs);
    }

    /**
     * @brief Less-than comparison operator
     * @param rhs Right-hand side Radians object
     * @return True if this angle is less than rhs
     */
    constexpr bool operator<(const Radians &rhs) const
    {
        return (this->value < rhs.value);
    }

    /**
     * @brief Greater-than comparison operator
     * @param rhs Right-hand side Radians object
     * @return True if this angle is greater than rhs
     */
    constexpr bool operator>(const Radians &rhs) const
    {
        return (this->value > rhs.value);
    }

    /**
     * @brief Less-than-or-equal comparison operator
     * @param rhs Right-hand side Radians object
     * @return True if this angle is less than or equal to rhs
     */
    constexpr bool operator<=(const Radians &rhs) const
    {
        return ((*this == rhs) || (*this < rhs));
    }

    /**
     * @brief Greater-than-or-equal comparison operator
     * @param rhs Right-hand side Radians object
     * @return True if this angle is greater than or equal to rhs
     */
    constexpr bool operator>=(const Radians &rhs) const
    {
        return ((*this == rhs) || (*this > rhs));
    }

    //=== Addition/Subtraction Operators ===//

    /**
     * @brief Addition operator
     * @param rhs Right-hand side Radians object
     * @return New Radians object with the sum of the angles
     */
    constexpr Radians operator+(const Radians &rhs) const
    {
        return Radians{ this->value + rhs.value };
    }

    /**
     * @brief Subtraction operator
     * @param rhs Right-hand side Radians object
     * @return New Radians object with the difference of the angles
     */
    constexpr Radians operator-(const Radians &rhs) const
    {
        return Radians{ this->value - rhs.value };
    }

    /**
     * @brief Addition assignment operator
     * @param rhs Right-hand side Radians object
     * @return Reference to this object after addition
     */
    constexpr Radians &operator+=(const Radians &rhs)
    {
        value += rhs.value;
        return *this;
    }

    /**
     * @brief Subtraction assignment operator
     * @param rhs Right-hand side Radians object
     * @return Reference to this object after subtraction
     */
    constexpr Radians &operator-=(const Radians &rhs)
    {
        value -= rhs.value;
        return *this;
    }

    //=== Arithmetic Operators ===//

    /**
     * @brief Unary plus operator
     * @return Copy of this object
     */
    constexpr Radians operator+() const
    {
        return *this;
    }

    /**
     * @brief Unary minus operator
     * @return New Radians object with negated angle
     */
    constexpr Radians operator-() const
    {
        return Radians{ -this->value };
    }

    //=== Multiplication/Division Operators ===//

    /**
     * @brief Multiplication by integer operator
     * @param rhs Integer multiplier
     * @return New Radians object with the angle multiplied by rhs
     */
    constexpr Radians operator*(const int &rhs) const
    {
        return Radians{ this->value * rhs };
    }

    /**
     * @brief Multiplication by double operator
     * @param rhs Double multiplier
     * @return New Radians object with the angle multiplied by rhs
     */
    constexpr Radians operator*(const double &rhs) const
    {
        return Radians{ this->value * rhs };
    }

    /**
     * @brief Division by integer operator
     * @param rhs Integer divisor
     * @return New Radians object with the angle divided by rhs
     */
    constexpr Radians operator/(const int &rhs) const
    {
        return Radians{ this->value / rhs };
    }

    /**
     * @brief Division by double operator
     * @param rhs Double divisor
     * @return New Radians object with the angle divided by rhs
     */
    constexpr Radians operator/(const double &rhs) const
    {
        return Radians{ this->value / rhs };
    }

    /**
     * @brief Multiplication assignment by integer operator
     * @param rhs Integer multiplier
     * @return Reference to this object after multiplication
     */
    constexpr Radians &operator*=(const int &rhs)
    {
        value *= rhs;
        return *this;
    }

    /**
     * @brief Multiplication assignment by double operator
     * @param rhs Double multiplier
     * @return Reference to this object after multiplication
     */
    constexpr Radians &operator*=(const double &rhs)
    {
        value *= rhs;
        return *this;
    }

    /**
     * @brief Division assignment by integer operator
     * @param rhs Integer divisor
     * @return Reference to this object after division
     */
    constexpr Radians &operator/=(const int &rhs)
    {
        value /= rhs;
        return *this;
    }

    /**
     * @brief Division assignment by double operator
     * @param rhs Double divisor
     * @return Reference to this object after division
     */
    constexpr Radians &operator/=(const double &rhs)
    {
        value /= rhs;
        return *this;
    }

    //=== Utility Methods ===//

    /**
     * @brief Normalize angle to a specific range
     * @param min Minimum value of the range (default: 0.0)
     * @param max Maximum value of the range (default: 2π)
     * @return Reference to this object after normalization
     *
     * Adjusts the angle to be within the specified range [min, max)
     */
    Radians &Normalize(double min = 0.0, double max = 2 * M_PI)
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
    double value{ 0.0 }; ///< The angle value in radians
};

} // namespace shkwon::unit

/**
 * @brief User-defined literal for creating Radians objects from floating-point values
 * @param value Angle in radians as a long double
 * @return Radians object initialized with the specified value
 */
constexpr shkwon::unit::Radians operator""_rad(long double value)
{
    return shkwon::unit::Radians{ static_cast<double>(value) };
}

/**
 * @brief User-defined literal for creating Radians objects from integer values
 * @param value Angle in radians as an unsigned long long
 * @return Radians object initialized with the specified value
 */
constexpr shkwon::unit::Radians operator""_rad(unsigned long long int value)
{
    return shkwon::unit::Radians{ static_cast<double>(value) };
}
