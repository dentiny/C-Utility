#include "exprops2.hpp" // operator overload
#include "exprarray.hpp" // Array definition

int main()
{
    Array<double> arr1(1000), arr2(1000);
    auto res = 1.2 * arr1 + arr1 * arr2;
}   