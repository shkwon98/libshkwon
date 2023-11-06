#pragma once

#include "status.hpp"

namespace shkwon
{
template <typename ReturnType>
struct [[nodiscard]] Result
{
    ReturnType data;
    ::shkwon::Status status;

    /**
     * Result can be used directly in if statements to check if it was successful.
     */
    operator bool() const
    {
        return status;
    }

    /**
     * Conditionally move the data into output, if status == true.
     * The Result should not be used after calling this function, as the status and data
     * may have been moved away.
     */
    ::shkwon::Status &&move_to(ReturnType *output)
    {
        if (status)
        {
            *output = std::move(data);
        }
        return std::move(status);
    }

    /**
     * Helper for moving out the data.
     * Usage example:
     * auto result = function_that_returns_result();
     * if (result){
     *     auto value = result.move();
     *     ... use value here ...
     * }
     *
     * The Result should not be used after calling this function, as the data
     * may have been moved away.
     */
    ReturnType &&move()
    {
        return std::move(data);
    }

    /**
     * Used to explicitly ignore any error present to silence nodiscard warnings.
     */
    inline void IgnoreError() const
    {
    }
};

} // namespace shkwon