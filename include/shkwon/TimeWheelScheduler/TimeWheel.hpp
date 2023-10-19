#pragma once

#include <chrono>
#include <list>
#include <string>
#include <vector>

#include "TimeWheelScheduler/TimeoutJob.hpp"

namespace ksh
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
     * @param totalSlotNumber The total number of slots in the TimeWheel.
     * @param interval The interval (in milliseconds) between each slot.
     * @param name An optional name for the TimeWheel.
     */
    TimeWheel(uint32_t totalSlotNumber, uint32_t interval, const std::string &name = "")
        : name(name)
        , slotNum(totalSlotNumber)
        , intervalInMillisecond(interval)
        , curSlotIdx(0)
        , slots(totalSlotNumber)
        , lessLevelTimeWheel(nullptr)
        , greaterLevelTimeWheel(nullptr)
    {}

    /**
     * Sets the less level time wheel for this time wheel.
     *
     * @param timeWheel A pointer to the less level time wheel to set.
     */
    void SetLessLevelTimeWheel(TimeWheel *timeWheel)
    {
        this->lessLevelTimeWheel = timeWheel;
    }

    /**
     * Sets the greater level time wheel for this time wheel.
     *
     * @param timeWheel A pointer to the greater level time wheel to set.
     */
    void SetGreaterLevelTimeWheel(TimeWheel *timeWheel)
    {
        this->greaterLevelTimeWheel = timeWheel;
    }

    /**
     * Returns the current time in milliseconds.
     *
     * @return The current time in milliseconds.
     */
    int64_t GetCurrentTime(void) const
    {
        int64_t time = this->curSlotIdx * this->intervalInMillisecond;
        if (this->lessLevelTimeWheel != nullptr)
        {
            time += this->lessLevelTimeWheel->GetCurrentTime();
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
        int64_t lessLevelTime = 0;
        if (this->lessLevelTimeWheel != nullptr)
        {
            lessLevelTime = this->lessLevelTimeWheel->GetCurrentTime();
        }
        auto diff = timer->GetExpirationTime() + lessLevelTime - GetNowTimestamp();

        // If the difference is greater than scale unit, the timer can be added into the current time wheel.
        if (diff >= this->intervalInMillisecond)
        {
            size_t n = (this->curSlotIdx + diff / this->intervalInMillisecond) % this->slotNum;
            this->slots[n].push_back(timer);
            return;
        }

        // If the difference is less than scale uint, the timer should be added into less level time wheel.
        if (this->lessLevelTimeWheel != nullptr)
        {
            this->lessLevelTimeWheel->AddTimer(timer);
            return;
        }

        // If the current time wheel is the least level, the timer can be added into the current time wheel.
        this->slots[this->curSlotIdx].push_back(timer);
    }

    /**
     * Increases the current time slot of the time wheel by one.
     */
    void Increase(void)
    {
        this->curSlotIdx++;
        if (this->curSlotIdx < this->slotNum)
        {
            return;
        }

        // If the time wheel is full, the greater level time wheel should be increased.
        // The timers in the current slot of the greater level time wheel should be moved into the current level time wheel.
        this->curSlotIdx %= this->slotNum;
        if (this->greaterLevelTimeWheel != nullptr)
        {
            this->greaterLevelTimeWheel->Increase();
            auto slot = std::move(this->greaterLevelTimeWheel->PopCurrentSlot());
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
        auto slot = std::move(slots[curSlotIdx]);
        return slot;
    }


private:
    std::string name;

    uint32_t slotNum;
    uint32_t intervalInMillisecond;
    uint32_t curSlotIdx;
    std::vector<std::list<TimerPtr>> slots;

    TimeWheel *lessLevelTimeWheel;
    TimeWheel *greaterLevelTimeWheel;
};

using TimeWheelPtr = std::shared_ptr<TimeWheel>;
} // namespace ksh