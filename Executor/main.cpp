#include "executor.hpp"
#include "constants.hpp"
#include <iostream>

void f1(int * val) { std::cout << "f1" << ' ' << *val << std::endl; }
void f2() { std::cout << "f2" << std::endl; }

int main()
{
    int * val = new int(10);
    Executor executor1 { f1, FuncType::VOID_INT_TYPE, val };
    Executor executor2 { f2, FuncType::VOID_VOID_TYPE };
    executor1.execute();
    executor2.execute();
}