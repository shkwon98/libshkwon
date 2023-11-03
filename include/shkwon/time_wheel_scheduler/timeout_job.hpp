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
    TimeoutJob(uint32_t id, int64_t when, int64_t interval, const TimerTask &task)
        : id_(id)
        , when_(when)
        , interval_(interval)
        , is_repeated_(interval > 0)
        , task_(task)
    {
    }

    void Run(void) const
    {
        if (task_)
        {
            task_();
        }
    }

    uint32_t GetID() const
    {
        return id_;
    }

    int64_t GetExpirationTime() const
    {
        return when_;
    }

    bool IsRepeated() const
    {
        return is_repeated_;
    }

    void UpdateExpirationTime(int64_t new_when = 0)
    {
        if (new_when > 0)
        {
            when_ = new_when;
        }
        else
        {
            when_ += interval_;
        }
    }

private:
    uint32_t id_;
    int64_t when_;
    int64_t interval_;
    bool is_repeated_;
    TimerTask task_;
};

using TimerPtr = std::shared_ptr<TimeoutJob>;
} // namespace shkwon