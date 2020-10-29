#ifndef THREAD_QUEUE_HPP__
#define THREAD_QUEUE_HPP__

#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>

template<typename T>
class Threadsafe_queue
{
public:
    Threadsafe_queue() {}

    Threadsafe_queue(const Threadsafe_queue & rhs)
    {
        std::lock_guard<std::mutex> lck(rhs.mtx);
        data_queue = rhs.data_queue;
    }

    Threadsafe_queue(Threadsafe_queue && rhs)
    {
        std::lock_guard<std::mutex> lck(rhs.mtx);
        data_queue = std::move(rhs.data_queue);
    }

    Threadsafe_queue & operator=(const Threadsafe_queue &) = delete;

    Threadsafe_queue & operator=(Threadsafe_queue &&) = delete;

    void push(T val)
    {
        std::shared_ptr<T> data(std::make_shared<T>(std::move(val)));
        std::lock_guard<std::mutex> lck(mtx);
        data_queue.push(data);
        cv.notify_one();
    }

    void wait_and_pop(T & val)
    {
        std::unique_lock<std::mutex> lck(mtx);
        cv.wait(lck, [this] { return !data_queue.empty(); });
        val = std::move(*data_queue.front());
        data_queue.pop();
    }

    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lck(mtx);
        cv.wait(lck, [this] { return !data_queue.empty(); });
        std::shared_ptr<T> res = data_queue.front();
        data_queue.pop();
        return res;
    }

    bool try_pop(T & val)
    {
        std::lock_guard<std::mutex> lxk(mtx);
        if(data_queue.empty())
        {
            return false;
        }

        val = std::move(*data_queue.front());
        data_queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lck(mtx);
        if(data_queue.empty())
        {
            return std::shared_ptr<T>();
        }

        std::shared_ptr<T> res = data_queue.front();
        data_queue.pop();
        return res;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lck(mtx);
        return data_queue.empty();
    }

private:
    mutable std::mutex mtx;
    std::condition_variable cv;
    std::queue<std::shared_ptr<T>> data_queue;
};

#endif // THREAD_QUEUE_HPP__