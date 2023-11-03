#include <chrono>
#include <iostream>
#include <mutex>
#include <set>
#include <thread>

template <typename T> class ExpirySet
{
public:
    ExpirySet(std::chrono::duration<int, std::milli> refresh_interval)
    {
        refreshing_thread_ = std::thread([this, refresh_interval]() {
            while (true)
            {
                std::this_thread::sleep_for(refresh_interval);
                this->Refresh();
            }
        });

        refreshing_thread_handle_ = refreshing_thread_.native_handle();
        refreshing_thread_.detach();
    }

    ExpirySet(const ExpirySet &left)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        data_ = left.data_;
    }
    ExpirySet &operator=(const ExpirySet &left)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        data_ = left.data_;
    }
    bool operator==(const ExpirySet &left)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        return data_ == left.data_;
    }
    bool operator!=(const ExpirySet &left)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        return data_ != left.data_;
    }

    ExpirySet(ExpirySet &&right)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        data_ = std::move(right.data_);
    }
    ExpirySet &operator=(ExpirySet &&right)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        data_ = std::move(right.data_);
    }
    bool operator==(ExpirySet &&right) = delete;
    bool operator!=(ExpirySet &&right) = delete;

    ~ExpirySet()
    {
        pthread_cancel(refreshing_thread_handle_);
    }

    void Insert(const T &value, const std::chrono::high_resolution_clock::time_point &expiration)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        data_.insert(std::make_pair(expiration, value));
    }
    void Insert(const T &value, const std::chrono::nanoseconds &timeout)
    {
        auto expiration_time = std::chrono::high_resolution_clock::now() + timeout;

        std::lock_guard<std::mutex> lock(mtx_);
        data_.insert(std::make_pair(expiration_time, value));
    }
    void Insert(const T &value, const std::chrono::microseconds &timeout)
    {
        auto expiration_time = std::chrono::high_resolution_clock::now() + timeout;

        std::lock_guard<std::mutex> lock(mtx_);
        data_.insert(std::make_pair(expiration_time, value));
    }
    void Insert(const T &value, const std::chrono::milliseconds &timeout)
    {
        auto expiration_time = std::chrono::high_resolution_clock::now() + timeout;

        std::lock_guard<std::mutex> lock(mtx_);
        data_.insert(std::make_pair(expiration_time, value));
    }
    void Insert(const T &value, const std::chrono::seconds &timeout)
    {
        auto expiration_time = std::chrono::high_resolution_clock::now() + timeout;

        std::lock_guard<std::mutex> lock(mtx_);
        data_.insert(std::make_pair(expiration_time, value));
    }
    void Insert(const T &value, const std::chrono::minutes &timeout)
    {
        auto expiration_time = std::chrono::high_resolution_clock::now() + timeout;

        std::lock_guard<std::mutex> lock(mtx_);
        data_.insert(std::make_pair(expiration_time, value));
    }
    void Insert(const T &value, const std::chrono::hours &timeout)
    {
        auto expiration_time = std::chrono::high_resolution_clock::now() + timeout;

        std::lock_guard<std::mutex> lock(mtx_);
        data_.insert(std::make_pair(expiration_time, value));
    }

    bool Contains(const T &value)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        for (auto it = data_.begin(); it != data_.end(); ++it)
        {
            if (it->second == value)
            {
                return true;
            }
        }
        return false;
    }

    void Remove(const T &value)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        for (auto it = data_.begin(); it != data_.end(); ++it)
        {
            if (it->second == value)
            {
                data_.erase(it);
                break;
            }
        }
    }

    void Refresh()
    {
        auto current_time = std::chrono::high_resolution_clock::now();

        std::lock_guard<std::mutex> lock(mtx_);
        while (!data_.empty() && data_.begin()->first <= current_time)
        {
            data_.erase(data_.begin());
        }
    }

private:
    std::set<std::pair<std::chrono::high_resolution_clock::time_point, T>> data_;
    std::mutex mtx_;
    std::thread refreshing_thread_;
    std::thread::native_handle_type refreshing_thread_handle_;
};
