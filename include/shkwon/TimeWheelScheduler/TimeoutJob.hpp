#pragma once

#include <functional>

namespace shkwon
{
typedef std::function<void()> TimerTask;

class TimeoutJob
{
public:
    /**
     * @brief Constructs a new TimeoutJob object with the specified ID, expiration time, interval, and task.
     *
     * @param id The ID of the timer.
     * @param when The time (in milliseconds) when the timer should expire.
     * @param interval The interval (in milliseconds) between repeated executions of the timer task.
     * @param task The task to be executed when the timer expires.
     */
    TimeoutJob(uint32_t id, int64_t when, int64_t interval, const TimerTask& task)
        : id(id)
        , when(when)
        , interval(interval)
        , isRepeated(interval > 0)
        , task(task)
    {}

    void Run(void) const
    {
        if (this->task)
        {
            this->task();
        }
    }

    uint32_t GetID() const
    {
        return this->id;
    }

    int64_t GetExpirationTime() const
    {
        return this->when;
    }

    bool IsRepeated() const
    {
        return this->isRepeated;
    }

    void UpdateExpirationTime()
    {
        this->when += this->interval;
    }

private:
    uint32_t id;
    int64_t when;
    int64_t interval;
    bool isRepeated;
    TimerTask task;
};

using TimerPtr = std::shared_ptr<TimeoutJob>;
} // namespace shkwon