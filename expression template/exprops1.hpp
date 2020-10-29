#ifndef EXPR_OPS1_HPP__
#define EXPR_OPS1_HPP__

#include "exprops1a.hpp"
#include <cstddef>
#include <cassert>

template<typename T, typename OP1, typename OP2>
class A_Add
{
public:
    A_Add(const OP1 & val1, const OP2 & val2) :
        op1 { val1 },
        op2 { val2 }
        {}

    T operator[](size_t idx) const { return op1[idx] + op2[idx]; }

    size_t size() const
    {
        constexpr std::size_t sz1 = op1.size();
        constexpr std::size_t sz2 = op2.size();
        assert(sz1 == 0 || sz2 == 0 || sz1 == sz2);
        return sz1 > 0 ? sz1 : sz2;
    }

private:
    typename A_Traits<OP1>::ExprRef op1; // first operand
    typename A_Traits<OP2>::ExprRef op2; // second operand
};

template<typename T, typename OP1, typename OP2>
class A_Mult
{
public:
    A_Mult(const OP1 & val1, const OP2 & val2) :
        op1 { val1 },
        op2 { val2 }
        {}

    T operator[](std::size_t idx) const { return op1[idx] * op2[idx]; }

    size_t size() const
    {
        constexpr std::size_t sz1 = op1.size();
        constexpr std::size_t sz2 = op2.size();
        assert(sz1 == 0 || sz2 == 0 || sz1 == sz2);
        return sz1 > 0 ? sz1 : sz2;
    }

private:
    typename A_Traits<OP1>::ExprRef op1; // first operand
    typename A_Traits<OP2>::ExprRef op2; // second operand
};

#endif // EXPR_OPS1_HPP__