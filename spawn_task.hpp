#ifndef SPAWN_TASK_HPP__
#define SPAWN_TASK_HPP__

#include <future>
#include <type_traits>

template<typename Func, typename ... Args>
std::future<std::result_of_t<Func(Args...)>> spawn_task(Func && func, Args && ... args)
{
    using result_type = std::result_of_t<Func(Args...)>;
    std::packaged_task<result_type(Args && ...)> task(std::move(func));
    std::future<result_type> fut(task.get_future());
    std::thread thr(std::move(task), std::move(args)...);
    thr.detach();
    return fut;
}

#endif // SPAWN_TASK_HPP__