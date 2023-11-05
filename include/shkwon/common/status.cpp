#pragma once

#include <sstream>
#include <string>
#include <system_error>

enum class SuccessCondition
{
    Success = 0
};
enum class SDKErrorCode
{
    Success = 0,
    InvalidArgument = 1,
};

namespace std
{
template <>
struct is_error_condition_enum<SuccessCondition> : true_type
{
};
template <>
struct is_error_code_enum<SDKErrorCode> : true_type
{
};
} // namespace std

namespace // anonymous namespace
{

struct SuccessConditionCategory : public std::error_category
{
public:
    const char *name() const noexcept override
    {
        return "SuccessCondition";
    }
    std::string message(int value) const override
    {
        switch (static_cast<SuccessCondition>(value))
        {
        case SuccessCondition::Success:
            return "Success";
        default:
            return "Fail";
        }
    }
};

struct SDKErrorCodeCategory : std::error_category
{
    const char *name() const noexcept override
    {
        return "SDKErrorCode";
    }
    std::string message(int value) const override
    {
        switch (static_cast<SDKErrorCode>(value))
        {
        case SDKErrorCode::Success:
            return "Success";
        case SDKErrorCode::InvalidArgument:
            return "InvalidArgument";
        }
        return "(SDKErrorCode: unrecognized error)";
    }
    bool equivalent(int valcode, const std::error_condition &cond) const noexcept override
    {
        if (cond == SuccessCondition::Success)
            return (valcode == 0);
        // if (cond == ErrorTypeCondition::SDKError)
        //     return true;
        return false;
    }
};

} // anonymous namespace

/*****************************************************************************/

const SuccessConditionCategory SuccessConditionCategory_category{};
const SDKErrorCodeCategory SDKErrorCodeCategory_category{};

std::error_condition make_error_condition(SuccessCondition value)
{
    return { static_cast<int>(value), SuccessConditionCategory_category };
}
std::error_code make_error_code(SDKErrorCode value)
{
    return { static_cast<int>(value), SDKErrorCodeCategory_category };
}

/*****************************************************************************/

namespace shkwon
{
namespace common
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

    operator bool() const
    {
        // ::shkwon::common::Status is the opposite of an std::error_code, meaning if error_code is set to
        // success/0, which casts to false, ::shkwon::common::Status will be set to true. This makes it more
        // intuitive if it is used as the return type of a method
        return (code_ == SuccessCondition::Success);
    }

    std::error_code code() const
    {
        return code_;
    }

    // Check if this matches a certain error enum.
    template <typename ENUM>
    bool Is() const
    {
        static_assert(std::is_error_code_enum<ENUM>::value, "Must check against an error code enum");
        return std::error_code(ENUM{}).category() == code_.category();
    }

    const std::string &message() const
    {
        return message_;
    }

    // Extend a ::shkwon::common::Status with a new message.
    ::shkwon::common::Status Chain(std::string message) const
    {
        message += ": ";
        message += message_;
        return Status(code_, std::move(message));
    }

    // Transform a ::shkwon::common::Status into a new code.
    ::shkwon::common::Status Chain(std::error_code code, std::string message) const
    {
        message += ": ";
        message += DebugString();
        return Status(code, std::move(message));
    }

    std::string DebugString() const
    {
        std::string result = std::to_string(code_.value());
        result += "(";
        result += code_.message();
        result += "): ";
        result += message_;
        return result;
    }

    // Used to explicitly ignore any error present to silence nodiscard warnings.
    inline void IgnoreError() const
    {
    }

private:
    std::error_code code_;
    std::string message_;
};

} // namespace common
} // namespace shkwon

/*****************************************************************************/

#include <iostream>

::shkwon::common::Status function1(int a)
{
    if (a == 0)
    {
        return ::shkwon::common::Status(SDKErrorCode::InvalidArgument, "a is 0");
        // return ::shkwon::common::Status(make_error_code(SDKErrorCode::InvalidArgument), "a is 0");
    }

    return ::shkwon::common::Status(SDKErrorCode::Success);
}

::shkwon::common::Status function2(int a)
{
    auto status = function1(a);
    if (!status)
    {
        return status.Chain("function1 failed");
    }

    return ::shkwon::common::Status(SDKErrorCode::Success);
}

::shkwon::common::Status function3(int a)
{
    auto status = function2(a);
    if (!status)
    {
        return status.Chain("function2 failed");
    }

    return ::shkwon::common::Status(SDKErrorCode::Success);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <int>\n", argv[0]);
        return 1;
    }

    auto a = std::atoi(argv[1]);

    auto status = function3(a);
    if (!status)
    {
        // std::cout << status.Is<SuccessCondition>() << std::endl;
        std::cerr << status.DebugString() << std::endl;
        return 1;
    }

    return 0;
}