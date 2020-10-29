// operator overload

#ifndef EXPR_OPS2_HPP__
#define EXPR_OPS2_HPP__

#include "exprops1.hpp" // A_Add, A_Mult
#include "exprarray.hpp" // Array
#include "exprscalar.hpp" // A_Scalar

// addition of two Arrays
template<typename T, typename R1, typename R2>
Array<T, A_Add<T, R1, R2>> operator+(const Array<T, R1> & arr1, const Array<T, R2> & arr2)
{
    return Array<T, A_Add<T, R1, R2>>(A_Add<T, R1, R2>(arr1.rep(), arr2.rep()));
}

// multiplication of two Arrays
template<typename T, typename R1, typename R2>
Array<T, A_Mult<T, R1, R2>> operator*(const Array<T, R1> & arr1, const Array<T, R2> & arr2)
{
    return Array<T, A_Mult<T, R1, R2>>(A_Mult<T, R1, R2>(arr1.rep(), arr2.rep()));
}

// multiplication between scalar and Array
template<typename T, typename R2>
Array<T, A_Mult<T, A_Scalar<T>, R2>> operator*(const T & val, const Array<T, R2> & arr)
{
    return Array<T, A_Mult<T, A_Scalar<T>, R2>>(A_Mult<T, A_Scalar<T>, R2>(A_Scalar<T>(val), arr.rep()));
}

template<typename T, typename R1>
Array<T, A_Mult<T, R1, A_Scalar<T>>> operator*(const Array<T, R1> & arr, const T & val)
{
    return Array<T, A_Mult<T, R1, A_Scalar<T>>>(A_Mult<T, R1, A_Scalar<T>>(arr.rep(), A_Scalar<T>(val)));
}

#endif