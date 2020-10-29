// helper traits class to select how to refer to an expression template node
// - in general by reference
// - for scalars by value

#ifndef EXPR_OPS_TRAITS_HPP__
#define EXPR_OPS_TRAITS_HPP__

#include "exprscalar.hpp"

template<typename T> class A_Scalars;

// primary template for non-scalar
template<typename T>
class A_Traits
{
public:
    using ExprRef = const T &;
};

// partial specialization for scalar
template<typename T>
class A_Traits<A_Scalars<T>>
{
public:
    using ExprRef = A_Scalars<T>;
};

#endif // EXPR_OPS_TRAITS_HPP__