#pragma once

#include <chrono>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <vector>

#include "shkwon/thread_pool/thread_pool.hpp"
#include "shkwon/time_wheel_scheduler/time_wheel.hpp"

namespace shkwon
{
class TimeWheelScheduler
{
public:
    /**
     * Constructs a TimeWheelScheduler object with a given timer step in milliseconds.
     *
     * @param interval The minimum time interval in milliseconds between two consecutive
     *                 executions of the timer wheel. The value must be greater than or
     *                 equal to 1ms and defaults to 50ms.
     */
    explicit TimeWheelScheduler(uint32_t interval = 50)
        : stop_flag_(false)
        , timer_id_(1)
        , interval_in_millisecond_(interval)
    {
        if (interval_in_millisecond_.count() < 1)
        {
            throw std::invalid_argument("TimeoutJob step must be greater than or equal to 10ms.");
        }
    }

    /**
     * Appends a new time wheel to the scheduler with the specified number of slots and interval.
     *
     * @param total_slot_num The total number of slots in the time wheel.
     * @param interval The interval (in milliseconds) between each slot in the time wheel.
     * @param name An optional name for the time wheel.
     */
    void AppendTimeWheel(uint32_t total_slot_num, uint32_t interval, const std::string &name = "")
    {
        auto curr_timewheel = std::make_shared<TimeWheel>(total_slot_num, interval, name);
        if (timewheels_.empty())
        {
            timewheels_.push_back(curr_timewheel);
            return;
        }

        auto greater_timewheel = timewheels_.back();
        greater_timewheel->SetLessLevelTimeWheel(curr_timewheel.get());
        curr_timewheel->SetGreaterLevelTimeWheel(greater_timewheel.get());
        timewheels_.push_back(curr_timewheel);
    }

    /**
     * Creates a timer that will execute the given task at the specified time.
     *
     * @param when The time at which the task will be executed in milliseconds since the Epoch.
     * @param task The task to execute when the timer expires.
     *
     * @return The ID of the newly created timer. Return 0 if the timer creation fails.
     */
    uint32_t CreateTimerAt(int64_t when, const TimerTask &task)
    {
        if (timewheels_.empty())
        {
            return 0;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        this->GetGreatestTimeWheel()->AddTimer(std::make_shared<TimeoutJob>(timer_id_, when, 0, task));

        return timer_id_++;
    }

    /**
     * Creates a timer that will execute the given task after the specified number of milliseconds.
     *
     * @param delay The time in milliseconds to wait before executing the task.
     * @param task The task to execute when the timer expires.
     *
     * @return The ID of the newly created timer. Return 0 if the timer creation fails.
     */
    uint32_t CreateTimerAfter(int64_t delay, const TimerTask &task)
    {
        auto when = GetNowTimestamp() + delay;
        return this->CreateTimerAt(when, task);
    }

    /**
     * Creates a timer that executes the given task every specified number of milliseconds.
     *
     * @param interval The time in milliseconds between periodic task executions.
     * @param task The task to execute.
     *
     * @return The ID of the created timer. Return 0 if the timer creation fails.
     */
    uint32_t CreateTimerEvery(int64_t interval, const TimerTask &task)
    {
        if (timewheels_.empty())
        {
            return 0;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        auto when = GetNowTimestamp() + interval;
        this->GetGreatestTimeWheel()->AddTimer(std::make_shared<TimeoutJob>(timer_id_, when, interval, task));

        return timer_id_++;
    }

    /**
     * Resets the timer with the given ID to expire at the specified time.
     *
     * @param id The ID of the timer to reset.
     * @param when The time at which the timer should expire, in milliseconds since epoch.
     */
    void ResetTimerAt(uint32_t id, int64_t when)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        restart_timer_infos_[id] = when;
    }

    /**
     * Resets the timer with the given ID to expire after the specified delay.
     *
     * @param id The ID of the timer to reset.
     * @param delay The time in milliseconds to wait before the timer expires.
     */
    void ResetTimerAfter(uint32_t id, int64_t delay)
    {
        auto when = GetNowTimestamp() + delay;
        this->ResetTimerAt(id, when);
    }

    /**
     * Cancels a timer with the given ID.
     *
     * @param id The ID of the timer to cancel.
     */
    void CancelTimer(uint32_t id)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        canceled_timer_ids_.insert(id);
    }

    /**
     * Starts the TimeWheelScheduler and begins executing scheduled tasks.
     *
     * @return true if the TimeWheelScheduler was successfully started, false otherwise.
     */
    bool Start(void)
    {
        if (timewheels_.empty())
        {
            return false;
        }

        thread_pool_ = std::make_unique<ThreadPool>(10);
        thread_ = std::thread([this]() {
            while (true)
            {
                auto now = std::chrono::system_clock::now();

                {
                    std::lock_guard<std::mutex> lock(mutex_);

                    if (stop_flag_)
                    {
                        break;
                    }

                    auto least_timewheel = this->GetLeastTimeWheel();
                    least_timewheel->Increase();
                    auto slot = std::move(least_timewheel->PopCurrentSlot());
                    for (const auto &timer : slot)
                    {
                        auto it = restart_timer_infos_.find(timer->GetID());
                        if (it != restart_timer_infos_.end())
                        {
                            timer->UpdateExpirationTime(it->second);
                            this->GetGreatestTimeWheel()->AddTimer(timer);
                            restart_timer_infos_.erase(it);
                            continue;
                        }

                        auto id = canceled_timer_ids_.find(timer->GetID());
                        if (id != canceled_timer_ids_.end())
                        {
                            canceled_timer_ids_.erase(id);
                            continue;
                        }

                        thread_pool_->Push([timer]() { timer->Run(); });

                        if (timer->IsRepeated())
                        {
                            timer->UpdateExpirationTime();
                            this->GetGreatestTimeWheel()->AddTimer(timer);
                        }
                    }
                }

                std::this_thread::sleep_until(now + interval_in_millisecond_);
            }
        });

        return true;
    }

    /**
     * Stops the time wheel scheduler, preventing any further tasks from being executed.
     */
    void Stop(void)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_flag_ = true;
        }

        thread_.join();
    }

private:
    TimeWheelPtr GetGreatestTimeWheel()
    {
        if (timewheels_.empty())
        {
            return TimeWheelPtr();
        }
        return timewheels_.front();
    }
    TimeWheelPtr GetLeastTimeWheel()
    {
        if (timewheels_.empty())
        {
            return TimeWheelPtr();
        }
        return timewheels_.back();
    }

    std::mutex mutex_;
    std::thread thread_;
    std::unique_ptr<ThreadPool> thread_pool_;

    bool stop_flag_;
    uint32_t timer_id_;
    std::chrono::milliseconds interval_in_millisecond_;

    std::vector<TimeWheelPtr> timewheels_;
    std::unordered_set<uint32_t> canceled_timer_ids_;
    std::unordered_map<uint32_t, int64_t> restart_timer_infos_;
};
} // namespace shkwon