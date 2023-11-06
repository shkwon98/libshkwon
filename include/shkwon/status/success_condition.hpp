#pragma once

#include <system_error>

enum class SuccessCondition
{
    Success = 0
};

namespace std
{
template <>
struct is_error_condition_enum<SuccessCondition> : true_type
{
};
} // namespace std

class SuccessConditionCategory : public std::error_category
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

const SuccessConditionCategory &success_condition_category{};
std::error_condition make_error_condition(SuccessCondition value) noexcept
{
    return std::error_condition{ static_cast<int>(value), success_condition_category };
}