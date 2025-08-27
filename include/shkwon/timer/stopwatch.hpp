#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>

namespace shkwon
{

/**
 * @class StopWatch
 * @brief A utility class for measuring elapsed time between operations
 *
 * @details StopWatch provides a simple mechanism to measure elapsed time between
 * a starting point (Tick) and an ending point (Tock). It's designed for performance
 * profiling and benchmarking purposes.
 */
class StopWatch
{
public:
    /**
     * @brief Start measuring time from this point
     */
    void Tick()
    {
        tick_ = GetElapsedTime();
    }

    /**
     * @brief Stop measuring time and return elapsed seconds since Tick()
     *
     * @return double Elapsed time in seconds
     */
    [[nodiscard]] double SilentTock() const
    {
        double tock = GetElapsedTime();
        return tock - tick_;
    }

    /**
     * @brief Stop measuring time and print the elapsed time
     *
     * @param prefix Optional string to prefix the timing output
     */
    void Tock(std::string_view prefix = "") const
    {
        Print(prefix, SilentTock());
    }

private:
    /**
     * @brief Helper function to print elapsed time
     *
     * @param prefix String to prefix the timing output
     * @param dt Elapsed time in seconds
     */
    static void Print(std::string_view prefix, const double dt)
    {
        if (!prefix.empty())
        {
            std::cout << prefix << "\t" << std::setprecision(4) << dt << " sec\n";
        }
        else
        {
            std::cout << "    \t" << std::setprecision(4) << dt << " sec\n";
        }
    }

    /**
     * @brief Get the current elapsed time since the reference point
     *
     * @return double Time in seconds since reference_time
     */
    static double GetElapsedTime()
    {
        std::chrono::steady_clock::time_point this_time = std::chrono::steady_clock::now();
        return std::chrono::duration<double>(this_time - reference_time).count();
    }

    static const std::chrono::steady_clock::time_point reference_time; ///< Reference time for elapsed time calculations
    double tick_{ 0.0 };                                               ///< Stores the elapsed time since the last Tick
};

/** @brief Reference time for elapsed time calculations */
inline const std::chrono::steady_clock::time_point StopWatch::reference_time = std::chrono::steady_clock::now();

} // namespace shkwon