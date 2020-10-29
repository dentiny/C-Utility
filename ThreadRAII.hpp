#ifndef THREAD_RAII_HPP__
#define THREAD_RAII_HPP__

#include <thread>

class ThreadRAII
{
public:
    enum class ThreadDtorStatus
    {
        JOIN,
        DETACH
    };

private:

public:
    ThreadRAII(std::thread && thd, ThreadDtorStatus dtorStat) :
        thd_ { std::move(thd) },
        dtorStat_ { dtorStat }
        {}

    ThreadRAII(const ThreadRAII & thd) = delete;

    ThreadRAII & operator=(const ThreadRAII & thd) = delete;

    ThreadRAII(ThreadRAII && thd) = default;

    ThreadRAII & operator=(ThreadRAII && thd) = default;

    std::thread & getThread() { return thd_; }

    ~ThreadRAII() noexcept
    {
        if(thd_.joinable())
        {
            if(dtorStat_ == ThreadDtorStatus::JOIN)
            {
                thd_.join();
            }
            else
            {
                thd_.detach();
            }
        }
    }

private:
    std::thread thd_;
    ThreadDtorStatus dtorStat_;
};

#endif