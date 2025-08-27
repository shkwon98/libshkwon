#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>

namespace shkwon
{

/**
 * @class ScopedTimer
 * @brief A RAII-style timer that automatically measures and reports elapsed time when it goes out of scope
 *
 * @details ScopedTimer starts timing when constructed and automatically prints the elapsed
 * time when destructed. This makes it ideal for timing code blocks or function calls
 * without explicit start/stop calls.
 */
class ScopedTimer
{
public:
    /**
     * @brief Construct a new ScopedTimer and start measuring time
     *
     * @param prefix Text to display with the timing result (defaults to "Timer")
     */
    explicit ScopedTimer(std::string_view prefix = "Timer")
        : prefix_(prefix)
        , start_time_(std::chrono::steady_clock::now())
    {
    }

    /**
     * @brief Destructor - prints the elapsed time since construction
     */
    ~ScopedTimer()
    {
        const auto end_time = std::chrono::steady_clock::now();
        const auto duration = std::chrono::duration<double>(end_time - start_time_).count();

        if (!prefix_.empty())
        {
            std::cout << prefix_ << "\t" << std::setprecision(4) << duration << " sec\n";
        }
        else
        {
            std::cout << "    \t" << std::setprecision(4) << duration << " sec\n";
        }
    }

    // Delete copy and move operations to prevent multiple measurements
    ScopedTimer(const ScopedTimer &) = delete;
    ScopedTimer &operator=(const ScopedTimer &) = delete;
    ScopedTimer(ScopedTimer &&) = delete;
    ScopedTimer &operator=(ScopedTimer &&) = delete;

private:
    std::string prefix_;                               ///< Prefix for the timer output
    std::chrono::steady_clock::time_point start_time_; ///< Time point when the timer was started
};

} // namespace shkwon
