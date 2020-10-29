#ifndef EXECUTOR_HPP__
#define EXECUTOR_HPP__

#include "constants.hpp"
#include <vector>

class Executor
{
private:
    template<typename T>
    void init(T arg)
    {
        argVec.push_back((void*)arg);
    }

    template<typename T, typename ... Args>
    void init(T arg, Args ... args)
    {
        argVec.push_back((void*)(arg));
        init(args...);
    }

public:
    template<typename Func>
    Executor(Func _f, FuncType type) : 
        f { (void*)(_f) },
        f_type { type }
        {}

    template<typename Func, typename ... Args>
    Executor(Func _f, FuncType type, Args ... args) : 
        f { (void*)(_f) },
        f_type { type }
    {
        init(args...);
    }

    void execute()
    {
        switch(f_type)
        {
            case FuncType::VOID_INT_TYPE:
            {
                auto temp_f1 = (void(*)(int*))f;
                temp_f1((int*)argVec[0]);
                break;
            }
            case FuncType::VOID_VOID_TYPE:
            {
                auto temp_f2 = (void(*)())f;
                temp_f2();
                break;
            }
            default:
                break;
        }
    }

    ~Executor() noexcept
    {
        for(void * arg : argVec)
        {
            delete(arg);
        }
    }

private:
    void * f;
    FuncType f_type;
    std::vector<void*> argVec;
};

#endif