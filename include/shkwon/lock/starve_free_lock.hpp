#pragma once

#include <condition_variable>
#include <mutex>

class StarveFreeLock
{
public:
    StarveFreeLock() noexcept
        : reader_count_(0)
        , writer_waiting_count_(0)
        , writing_(false)
    {
    }

    void lock_shared() noexcept
    {
        std::unique_lock<std::mutex> lock(mutex_);
        reader_cv_.wait(lock, [this]() { return writer_waiting_count_ == 0 && !writing_; });
        ++reader_count_;
    }

    void unlock_shared() noexcept
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (--reader_count_ == 0 && writer_waiting_count_ > 0)
        {
            writer_cv_.notify_one();
        }
    }

    void lock() noexcept
    {
        std::unique_lock<std::mutex> lock(mutex_);
        ++writer_waiting_count_;
        writer_cv_.wait(lock, [this]() { return reader_count_ == 0 && !writing_; });
        --writer_waiting_count_;
        writing_ = true;
    }

    void unlock() noexcept
    {
        std::unique_lock<std::mutex> lock(mutex_);
        writing_ = false;
        if (writer_waiting_count_ > 0)
        {
            writer_cv_.notify_one();
        }
        else
        {
            reader_cv_.notify_all();
        }
    }

private:
    std::mutex mutex_;
    std::condition_variable reader_cv_;
    std::condition_variable writer_cv_;
    int reader_count_;
    int writer_waiting_count_;
    bool writing_;
};
