#ifndef THREAD_SAFE_STACK_HPP__
#define THREAD_SAFE_STACK_HPP__

#include <mutex>
#include <stack>
#include <memory>
#include <exception>

class EmptyStack: std::exception
{
    virtual const char * what() const throw() 
    {
        return "Empty Stack";
    };
};

template<typename T>
class Threadsafe_stack
{
public:
    Threadsafe_stack() : 
        data { std::stack<T>() } 
        {}

    Threadsafe_stack(const Threadsafe_stack & rhs)
    {
        std::lock_guard<std::mutex> lck(rhs.mtx);
        data = rhs.data;
    }

    Threadsafe_stack(Threadsafe_stack && rhs)
    {
        std::lock_guard<std::mutex> lck(rhs.mtx);
        data = std::move(rhs.data);
    }

    Threadsafe_stack & operator=(const Threadsafe_stack&) = delete;

    Threadsafe_stack & operator=(Threadsafe_stack &&) = delete;

    void push(T val)
    {
        std::lock_guard<std::mutex> lck(mtx);
        data.push(val);
    }

    std::shared_ptr<T> pop()
    {
        std::lock_guard<std::mutex> lck(mtx);
        if(data.empty()) throw EmptyStack();

        std::shared_ptr<T> const res(std::make_shared<T>(data.top()));
        data.pop();
        return res;
    }

    void pop(T & val)
    {
        std::lock_guard<std::mutex> lck(mtx);
        if(data.empty()) throw EmptyStack();

        val = data.top();
        data.pop();
    }

    bool empty() const noexcept
    {
        std::lock_guard<std::mutex> lck(mtx);
        return data.empty();
    }

private:
    std::stack<T> data;
    mutable std::mutex mtx;
};

#endif // THREAD_SAFE_STACK_HPP__