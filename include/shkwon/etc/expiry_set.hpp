#include <iostream>
#include <chrono>
#include <mutex>
#include <set>
#include <thread>

template <typename T>
class expiry_set
{
public:
    expiry_set(std::chrono::duration<int, std::milli> refreshInterval)
    {
        this->refreshThread = std::thread([this, refreshInterval]()
        {
            while (true)
            {
                std::this_thread::sleep_for(refreshInterval);
                this->Refresh();
            }
        });

        this->refreshThreadHandle = refreshThread.native_handle();
        this->refreshThread.detach();
    }


    expiry_set(const expiry_set & left)
    {
        std::lock_guard<std::mutex> lock(mtx);
        this->data = left.data;
    }
    expiry_set & operator= (const expiry_set & left)
    {
        std::lock_guard<std::mutex> lock(mtx);
        this->data = left.data;
    }
    bool operator== (const expiry_set & left)
    {
        std::lock_guard<std::mutex> lock(mtx);
        return this->data == left.data;
    }
    bool operator!= (const expiry_set & left)
    {
        std::lock_guard<std::mutex> lock(mtx);
        return this->data != left.data;
    }

    expiry_set(expiry_set && right)
    {
        std::lock_guard<std::mutex> lock(mtx);
        this->data = std::move(right.data);
    }
    expiry_set & operator= (expiry_set && right)
    {
        std::lock_guard<std::mutex> lock(mtx);
        this->data = std::move(right.data);
    }
    bool operator== (expiry_set && right) = delete;
    bool operator!= (expiry_set && right) = delete;

    ~expiry_set()
    {
        pthread_cancel(this->refreshThreadHandle);
    }


    void Insert(const T & value, const std::chrono::high_resolution_clock::time_point & expiration)
    {
        std::lock_guard<std::mutex> lock(mtx);
        this->data.insert(std::make_pair(expiration, value));
    }
    void Insert(const T & value, const std::chrono::nanoseconds & timeout)
    {
        auto expirationTime = std::chrono::high_resolution_clock::now() + timeout;

        std::lock_guard<std::mutex> lock(mtx);
        this->data.insert(std::make_pair(expirationTime, value));
    }
    void Insert(const T & value, const std::chrono::microseconds & timeout)
    {
        auto expirationTime = std::chrono::high_resolution_clock::now() + timeout;

        std::lock_guard<std::mutex> lock(mtx);
        this->data.insert(std::make_pair(expirationTime, value));
    }
    void Insert(const T & value, const std::chrono::milliseconds & timeout)
    {
        auto expirationTime = std::chrono::high_resolution_clock::now() + timeout;

        std::lock_guard<std::mutex> lock(mtx);
        this->data.insert(std::make_pair(expirationTime, value));
    }
    void Insert(const T & value, const std::chrono::seconds & timeout)
    {
        auto expirationTime = std::chrono::high_resolution_clock::now() + timeout;

        std::lock_guard<std::mutex> lock(mtx);
        this->data.insert(std::make_pair(expirationTime, value));
    }
    void Insert(const T & value, const std::chrono::minutes & timeout)
    {
        auto expirationTime = std::chrono::high_resolution_clock::now() + timeout;

        std::lock_guard<std::mutex> lock(mtx);
        this->data.insert(std::make_pair(expirationTime, value));
    }
    void Insert(const T & value, const std::chrono::hours & timeout)
    {
        auto expirationTime = std::chrono::high_resolution_clock::now() + timeout;

        std::lock_guard<std::mutex> lock(mtx);
        this->data.insert(std::make_pair(expirationTime, value));
    }

    bool Contains(const T & value)
    {
        std::lock_guard<std::mutex> lock(mtx);
        for (auto it = this->data.begin(); it != this->data.end(); ++it)
        {
            if (it->second == value)
            {
                return true;
            }
        }
        return false;
    }

    void Remove(const T & value)
    {
        std::lock_guard<std::mutex> lock(mtx);
        for (auto it = this->data.begin(); it != this->data.end(); ++it)
        {
            if (it->second == value)
            {
                this->data.erase(it);
                break;
            }
        }
    }

    void Refresh()
    {
        auto currentTime = std::chrono::high_resolution_clock::now();

        std::lock_guard<std::mutex> lock(mtx);
        while (!data.empty() && this->data.begin()->first <= currentTime)
        {
            this->data.erase(data.begin());
        }
    }


private:
    std::set<std::pair<std::chrono::high_resolution_clock::time_point, T>> data;
    std::mutex mtx;
    std::thread refreshThread;
    std::thread::native_handle_type refreshThreadHandle;
};
