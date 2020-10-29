// class for objects that represent scalars

#ifndef EXPR_SCALARS_HPP__
#define EXPR_SCALARS_HPP__

#include <cstddef>

template<typename T>
class A_Scalar
{
public:
    constexpr A_Scalar(const T & _val) : 
        val { _val }
        {}

    constexpr const T & operator[] (std::size_t) const { return val; }
    
    constexpr std::size_t size() const { return 0; }

private:
    const T & val;
};

#endif // EXPR_SCALARS_HPP__