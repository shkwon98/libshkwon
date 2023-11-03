#include <chrono>
#include <iostream>
#include <thread>

namespace shkwon
{
class Timer
{
public:
    Timer()
    {
        start_ = std::chrono::high_resolution_clock::now();
    }
    ~Timer()
    {
        end_ = std::chrono::high_resolution_clock::now();
        duration_ = end_ - start_;

        auto ms = duration_.count() * 1000.0f;
        std::cout << "Timer took " << ms << "ms" << std::endl;
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_, end_;
    std::chrono::duration<double> duration_;
};
} // namespace shkwon