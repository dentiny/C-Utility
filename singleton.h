#ifndef SINGLETON_H_
#define SINGLETON_H_

#include <boost/noncopyable.hpp>
#include <mutex>

template<typename T>
class Singleton : boost::noncopyable {
public: 
    Singleton() = delete;

    static T& getInstance() {
        std::call_once(init_flag_, &Singleton::init);
        return *val_;
    }

private:
    static void init() {
        val_ = new T();
    }

private:
    static inline std::once_flag init_flag_{};
    static inline T* val_ = nullptr;
};

#endif // SINGLETON_H_