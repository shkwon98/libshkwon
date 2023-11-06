#pragma once

#include <string>

#include "success_condition.hpp"

namespace shkwon
{
class [[nodiscard]] Status
{
public:
    Status(const std::error_code &code, std::string message = "")
        : code_(code)
        , message_(std::move(message))
    {
    }
    Status() = default;
    ~Status() = default;

    /**
     * This operator returns true if the code_ member variable of the ::shkwon::Status object is set to
     * SuccessCondition::Success/0, which means that the operation was successful. This operator is useful when the
     * ::shkwon::Status object is used as the return type of a method, making it more intuitive to check if the
     * operation was successful or not.
     */
    operator bool() const
    {
        return (code_ == SuccessCondition::Success);
    }

    std::error_code code() const
    {
        return code_;
    }

    /**
     * Check if this matches a certain error code enum.
     *
     * @return true if the error code enum matches the error code enum of the ::shkwon::Status object.
     */
    template <typename ENUM>
    bool Is() const
    {
        static_assert(std::is_error_code_enum<ENUM>::value, "Must check against an error code enum");
        return std::error_code(ENUM{}).category() == code_.category();
    }

    /**
     * Check if this matches a certain error condition enum.
     *
     * @return true if the error condition enum matches the error condition enum of the ::shkwon::Status
     *         object.
     */
    template <typename ENUM>
    bool Equivalent(const std::error_condition &condition) const
    {
        static_assert(std::is_error_condition_enum<ENUM>::value, "Must check against an error condition enum");
        return code_.category().equivalent(static_cast<int>(code_.value()), condition);
    }

    /**
     * @return The message associated with the status. Recommends using DebugString() instead for more information.
     */
    const std::string &message() const
    {
        return message_;
    }

    /**
     * Extend a ::shkwon::Status with a new message.
     */
    ::shkwon::Status Chain(std::string message) const
    {
        message += " >> ";
        message += message_;
        return Status(code_, std::move(message));
    }

    /**
     * Transform a ::shkwon::Status into a new code.
     */
    ::shkwon::Status Chain(std::error_code code, std::string message) const
    {
        message += " >> ";
        message += DebugString();
        return Status(code, std::move(message));
    }

    /**
     * @return A string containing the error code, error message, and the message associated with the status.
     */
    std::string DebugString() const
    {
        std::string result = std::to_string(code_.value());
        result += "(";
        result += code_.message();
        result += "): ";
        result += message_;
        return result;
    }

    /**
     * Used to explicitly ignore any error present to silence nodiscard warnings.
     */
    inline void IgnoreError() const
    {
    }

private:
    std::error_code code_;
    std::string message_;
};

} // namespace shkwon
