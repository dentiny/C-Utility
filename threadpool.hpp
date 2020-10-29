#ifndef THREADPOOL_HPP__
#define THREADPOOL_HPP__

#include <mutex>
#include <queue>
#include <vector>
#include <thread>
#include <string>
#include <atomic>
#include <functional>
#include <condition_variable>

class ThreadPool
{
public:
    FixedThreadPool(unsigned pool_size) :
        stop { false },
        size { pool_size }
    {
        for(size_t i = 0; i < size; ++i)
        {
            threads.emplace_back
            (
                [&]()
                {
                    for(;;)
                    {
                        std::unique_lock<std::mutex> lck(mtx);
                        cv.wait(lck, [&]{ return stop || !requests.empty(); });
                        if(stop && requests.empty()) return;
                        Request * request = requests.front(); requests.pop();
                        lck.unlock(); // local variable doesn't need synchronziation protection
                        handleRequest(request);
                    }
                }
            );
        }
    }

    virtual void execute(Request * request) override
    {
        {
            std::unique_lock<std::mutex> lck(mtx);
            requests.push(request);
        }
        cv.notify_one();
    }

    virtual void destroy() noexcept override
    {
        stop = true;
        cv.notify_all();
        for(auto & thr : threads)
        {
            if(thr.joinable())
            {
                thr.join();
            }
        }
    }

private:
    std::atomic<bool> stop;
    std::atomic<unsigned> size;
    std::mutex mtx;
    std::condition_variable cv;
    std::vector<std::thread> threads;
    std::queue<Request*> requests; // parameters to execute
};

#endif