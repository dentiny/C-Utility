#ifndef SPINLOCK_HPP__
#define SPINLOCK_HPP__

#include <atomic>

class Spinlock
{
public:
    Spinlock() :
        flag { ATOMIC_FLAG_INIT }
        {}

    Spinlock(const Spinlock & rhs) = delete;

    Spinlock & operator=(const Spinlock & rhs) = delete;

    void lock()
    {
        while(flag.test_and_set(std::memory_order_acquire));
    }

    void unlock()
    {
        flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag;
};

#endif // SPINLOCK_HPP__