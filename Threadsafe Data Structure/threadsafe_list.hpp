#ifndef THREADSAFE_LIST_HPP__
#define THREADSAFE_LIST_HPP__

#include <mutex>
#include <memory>
#include <algorithm>

template<typename T>
class ThreadsafeList
{
private:
    struct Node
    {
        std::mutex mtx;
        std::shared_ptr<T> data;
        std::unique_ptr<Node> next;

        Node() = default;

        Node(const T & val) : 
            data { std::make_shared(val) }
            {}
    };

public:
    ThreadsafeList() = default;

    ThreadsafeList(const ThreadsafeList &) = delete;

    ThreadsafeList & operator=(const ThreadsafeList &) = delete;

    void push_front(const T & val)
    {
        std::unique_ptr<Node> new_node = std::make_unique(val);
        std::lock_guard<std::mutex> lck(head.mtx);
        new_node->next = std::move(head.next);
        head.next = std::move(new_node);
    }

    template<typename Func>
    void for_each(Func func)
    {
        Node * current = &head;
        std::unique_lock<std::mutex> lck(head.mtx);
        while(Node * const next = current->next.get())
        {
            std::unique_lock<std::mutex> next_lck(next->mtx);
            lck.unlock();
            func(*next->data);
            current = next;
            lck = std::move(next_lck);
        }
    }

    template<typename Predicate>
    std::shared_ptr<T> find_first_of(Predicate pred)
    {
        Node * current = &head;
        std::unique_lock<std::mutex> lck(head.mtx);
        while(Node * const next = current->next.get())
        {
            std::unique_lock<std::mutex> next_lck(next->mtx);
            lck.unlock();
            if(pred(*next->data))
            {
                return next->data;
            }
            current = next;
            lck = std::move(next_lck);
        }
        return std::shared_ptr<T>();
    }

    template<typename Predicate>
    void remove_if(Predicate pred)
    {
        Node * current = &head;
        std::unique_lock<std::mutex> lck(head.mtx);
        while(Node * const next = current->next.get())
        {
            std::unique_lock<std::mutex> next_lck(next->mtx);
            if(pred(*next->data))
            {
                std::unique_lock<Node> old_next = std::move(current->next);
                current->next = std::move(next->next);
                next_lck.unlock();
            }
            else
            {
                lck.unlock();
                current = next;
                lck = std::move(next_lck);
            }
        }
        return std::shared_ptr<T>();
    }

private:
    Node head;
};

#endif // THREADSAFE_LIST_HPP__