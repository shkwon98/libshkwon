#pragma once

#include <chrono>
#include <list>
#include <string>
#include <vector>

#include "shkwon/time_wheel_scheduler/timeout_job.hpp"

namespace shkwon
{
inline int64_t GetNowTimestamp()
{
    using namespace std::chrono;

    auto now = system_clock::now().time_since_epoch();
    return duration_cast<milliseconds>(now).count();
}

class TimeWheel
{
public:
    /**
     * Constructs a TimeWheel object with the specified number of slots and interval.
     *
     * @param total_slot_num The total number of slots in the TimeWheel.
     * @param interval The interval (in milliseconds) between each slot.
     * @param name An optional name for the TimeWheel.
     */
    TimeWheel(uint32_t total_slot_num, uint32_t interval, const std::string &name = "")
        : name_(name)
        , slot_num_(total_slot_num)
        , interval_in_millisecond_(interval)
        , curr_slot_idx_(0)
        , slots_(total_slot_num)
        , less_level_timewheel_(nullptr)
        , greater_level_timewheel_(nullptr)
    {
    }

    /**
     * Sets the less level time wheel for this time wheel.
     *
     * @param timewheel A pointer to the less level time wheel to set.
     */
    void SetLessLevelTimeWheel(TimeWheel *timewheel)
    {
        less_level_timewheel_ = timewheel;
    }

    /**
     * Sets the greater level time wheel for this time wheel.
     *
     * @param timewheel A pointer to the greater level time wheel to set.
     */
    void SetGreaterLevelTimeWheel(TimeWheel *timewheel)
    {
        greater_level_timewheel_ = timewheel;
    }

    /**
     * Returns the current time in milliseconds.
     *
     * @return The current time in milliseconds.
     */
    int64_t GetCurrentTime(void) const
    {
        int64_t time = curr_slot_idx_ * interval_in_millisecond_;
        if (less_level_timewheel_ != nullptr)
        {
            time += less_level_timewheel_->GetCurrentTime();
        }

        return time;
    }

    /**
     * Adds a timer to the time wheel.
     *
     * @param timer A shared pointer to the timer to add.
     */
    void AddTimer(TimerPtr timer)
    {
        int64_t less_level_time = 0;
        if (less_level_timewheel_ != nullptr)
        {
            less_level_time = less_level_timewheel_->GetCurrentTime();
        }
        auto diff = timer->GetExpirationTime() + less_level_time - GetNowTimestamp();

        // If the difference is greater than scale unit, the timer can be added into the current time wheel.
        if (diff >= interval_in_millisecond_)
        {
            size_t n = (curr_slot_idx_ + diff / interval_in_millisecond_) % slot_num_;
            slots_[n].push_back(timer);
            return;
        }

        // If the difference is less than scale uint, the timer should be added into less level time wheel.
        if (less_level_timewheel_ != nullptr)
        {
            less_level_timewheel_->AddTimer(timer);
            return;
        }

        // If the current time wheel is the least level, the timer can be added into the current time wheel.
        slots_[curr_slot_idx_].push_back(timer);
    }

    /**
     * Increases the current time slot of the time wheel by one.
     */
    void Increase(void)
    {
        curr_slot_idx_++;
        if (curr_slot_idx_ < slot_num_)
        {
            return;
        }

        // If the time wheel is full, the greater level time wheel should be increased.
        // The timers in the current slot of the greater level time wheel should be moved into the current level time
        // wheel.
        curr_slot_idx_ %= slot_num_;
        if (greater_level_timewheel_ != nullptr)
        {
            greater_level_timewheel_->Increase();
            auto slot = std::move(greater_level_timewheel_->PopCurrentSlot());
            for (const auto &timer : slot)
            {
                AddTimer(timer);
            }
        }
    }

    /**
     * Removes and returns all timers in the current time slot.
     *
     * @return A list of shared pointers to the timers that were removed.
     */
    std::list<TimerPtr> PopCurrentSlot()
    {
        auto slot = std::move(slots_[curr_slot_idx_]);
        return slot;
    }

private:
    std::string name_;

    uint32_t slot_num_;
    uint32_t interval_in_millisecond_;
    uint32_t curr_slot_idx_;
    std::vector<std::list<TimerPtr>> slots_;

    TimeWheel *less_level_timewheel_;
    TimeWheel *greater_level_timewheel_;
};

using TimeWheelPtr = std::shared_ptr<TimeWheel>;
} // namespace shkwon