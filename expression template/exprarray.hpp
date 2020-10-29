#ifndef EXPR_ARRAY_HPP__
#define EXPR_ARRAY_HPP__

#include "sarray1.hpp"
#include <cstddef>
#include <cassert>

template<typename T, typename Rep = SArray<T>>
class Array
{
public:
    explicit Array(std::size_t sz) : 
        expr_rep(sz)
        {}

    Array(const Rep & rhs) :
        expr_rep(rhs)
        {}

    // copy assignment for the same type
    Array & operator=(const Array & rhs)
    {
        assert(size() == rhs.size());
        if(this != &rhs)
        {
            for(std::size_t idx = 0; idx < rhs.size(); ++idx)
            {
                expr_rep[idx] = rhs[idx];
            }
        }
        return *this;
    }

    // copy assignment for different types
    template<typename T2, typename Rep2>
    Array & operator=(const Array<T2, Rep2> & rhs)
    {
        assert(size() == rhs.size());
        if(this != &rhs)
        {
            for(std::size_t idx = 0; idx < size(); ++idx)
            {
                expr_rep[idx] = rhs[idx];
            }
        }
        return *this;
    }

    std::size_t size() const noexcept { return expr_rep.size(); }

    decltype(auto) operator[](std::size_t idx) const 
    { 
        assert(idx < size());
        return expr_rep[idx];
    }

    T & operator[](std::size_t idx)
    {
        assert(idx < size());
        return expr_rep[idx];
    }

    const Rep & rep() const { return expr_rep; }

    Rep & rep() { return expr_rep; } 

private:
    Rep expr_rep;
};

#endif