#ifndef SARRAY_HPP__
#define SARRAY_HPP__

#include <cstddef>
#include <cassert>

template<typename T>
class SArray
{
protected:
    void init()
    {
        for(std::size_t idx = 0; idx < size(); ++idx)
        {
            storage[idx] = T{};
        }
    }

    void copy(const SArray<T> & rhs)
    {
        assert(size() == rhs.size());
        for(std::size_t idx = 0; idx < size(); ++idx)
        {
            storage[idx] = rhs.storage[idx];
        }
    }

public:
    explicit SArray(std::size_t sz) : 
        storage { new T[sz] },
        storage_size { sz }
    {
        init();
    }

    SArray(const SArray<T> & rhs) :
        storage { new T[rhs.size()] },
        storage_size { rhs.size() }
    {
        copy(rhs);        
    }

    ~SArray() noexcept { delete[] storage; }

    std::size_t size() const noexcept { return storage_size; }

    T & operator[](std::size_t idx) { return storage[idx]; }

    const T & operator[](std::size_t idx) const { return storage[idx]; }

private:
    T * storage;
    std::size_t storage_size;
};

#endif // SARRAY_HPP__