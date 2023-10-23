#pragma once

#include <chrono>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <vector>

#include "shkwon/TimeWheelScheduler/TimeWheel.hpp"
#include "shkwon/ThreadPool/ThreadPool.hpp"

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
        : stopFlag(false)
        , timerID(1)
        , intervalInMillisecond(interval)
    {
        if (this->intervalInMillisecond.count() < 1)
        {
            throw std::invalid_argument("TimeoutJob step must be greater than or equal to 10ms.");
        }
    }

    /**
     * Appends a new time wheel to the scheduler with the specified number of slots and interval.
     *
     * @param totalSlotNumber The total number of slots in the time wheel.
     * @param interval The interval (in milliseconds) between each slot in the time wheel.
     * @param name An optional name for the time wheel.
     */
    void AppendTimeWheel(uint32_t totalSlotNumber, uint32_t interval, const std::string &name = "")
    {
        auto curTimeWheel = std::make_shared<TimeWheel>(totalSlotNumber, interval, name);
        if (this->timeWheels.empty())
        {
            this->timeWheels.push_back(curTimeWheel);
            return;
        }

        auto greaterTimeWheel = this->timeWheels.back();
        greaterTimeWheel->SetLessLevelTimeWheel(curTimeWheel.get());
        curTimeWheel->SetGreaterLevelTimeWheel(greaterTimeWheel.get());
        this->timeWheels.push_back(curTimeWheel);
    }

    /**
     * Creates a timer that will execute the given task at the specified time.
     *
     * @param when The time at which the task will be executed in milliseconds since the Epoch.
     * @param task The task to execute when the timer expires.
     *
     * @return The ID of the newly created timer. Return 0 if the timer creation fails.
     */
    uint32_t CreateTimerAt(int64_t when, const TimerTask& task)
    {
        if (this->timeWheels.empty())
        {
            return 0;
        }

        std::lock_guard<std::mutex> lock(this->mutex);
        this->GetGreatestTimeWheel()->AddTimer(std::make_shared<TimeoutJob>(this->timerID, when, 0, task));

        return this->timerID++;
    }

    /**
     * Creates a timer that will execute the given task after the specified number of milliseconds.
     *
     * @param delay The time in milliseconds to wait before executing the task.
     * @param task The task to execute when the timer expires.
     *
     * @return The ID of the newly created timer. Return 0 if the timer creation fails.
     */
    uint32_t CreateTimerAfter(int64_t delay, const TimerTask& task)
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
    uint32_t CreateTimerEvery(int64_t interval, const TimerTask& task)
    {
        if (this->timeWheels.empty())
        {
            return 0;
        }

        std::lock_guard<std::mutex> lock(this->mutex);
        auto when = GetNowTimestamp() + interval;
        this->GetGreatestTimeWheel()->AddTimer(std::make_shared<TimeoutJob>(this->timerID, when, interval, task));

        return this->timerID++;
    }

    /**
     * Resets the timer with the given ID to expire at the specified time.
     *
     * @param id The ID of the timer to reset.
     * @param when The time at which the timer should expire, in milliseconds since epoch.
     */
    void ResetTimerAt(uint32_t id, int64_t when)
    {
        std::lock_guard<std::mutex> lock(this->mutex);
        this->restartTimerInfos[id] = when;
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
        std::lock_guard<std::mutex> lock(this->mutex);
        this->cancelTimerIds.insert(id);
    }

    /**
     * Starts the TimeWheelScheduler and begins executing scheduled tasks.
     *
     * @return true if the TimeWheelScheduler was successfully started, false otherwise.
     */
    bool Start(void)
    {
        if (this->timeWheels.empty())
        {
            return false;
        }

        this->threadPool = std::make_unique<ThreadPool>(10);
        this->thread = std::thread([this]()
        {
            while (true)
            {
                auto now = std::chrono::system_clock::now();

                {
                    std::lock_guard<std::mutex> lock(this->mutex);

                    if (this->stopFlag)
                    {
                        break;
                    }

                    auto leastTimeWheel = this->GetLeastTimeWheel();
                    leastTimeWheel->Increase();
                    auto slot = std::move(leastTimeWheel->PopCurrentSlot());
                    for (const auto &timer : slot)
                    {
                        auto it = this->restartTimerInfos.find(timer->GetID());
                        if (it != this->restartTimerInfos.end())
                        {
                            timer->UpdateExpirationTime(it->second);
                            this->GetGreatestTimeWheel()->AddTimer(timer);
                            this->restartTimerInfos.erase(it);
                            continue;
                        }

                        auto id = this->cancelTimerIds.find(timer->GetID());
                        if (id != this->cancelTimerIds.end())
                        {
                            this->cancelTimerIds.erase(id);
                            continue;
                        }

                        this->threadPool->Push([timer]()
                        {
                            timer->Run();
                        });

                        if (timer->IsRepeated())
                        {
                            timer->UpdateExpirationTime();
                            this->GetGreatestTimeWheel()->AddTimer(timer);
                        }
                    }
                }

                std::this_thread::sleep_until(now + this->intervalInMillisecond);
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
            std::lock_guard<std::mutex> lock(this->mutex);
            this->stopFlag = true;
        }

        this->thread.join();
    }

private:
    TimeWheelPtr GetGreatestTimeWheel()
    {
        if (this->timeWheels.empty())
        {
            return TimeWheelPtr();
        }
        return this->timeWheels.front();
    }
    TimeWheelPtr GetLeastTimeWheel()
    {
        if (this->timeWheels.empty())
        {
            return TimeWheelPtr();
        }
        return this->timeWheels.back();
    }

    std::mutex mutex;
    std::thread thread;
    std::unique_ptr<ThreadPool> threadPool;

    bool stopFlag;
    uint32_t timerID;
    std::chrono::milliseconds intervalInMillisecond;

    std::vector<TimeWheelPtr> timeWheels;
    std::unordered_set<uint32_t> cancelTimerIds;
    std::unordered_map<uint32_t, int64_t> restartTimerInfos;
};
} // namespace shkwon