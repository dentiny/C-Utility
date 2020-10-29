#include "minipool.h"
#include <iostream>

// __PRETTY_FUNCTION__ is a macro print the name of the function/method
#define TRACE_METHOD() std::cout << this << " " << __PRETTY_FUNCTION__ << "\n";

struct Foo 
{
    int x = 42;
    Foo() { TRACE_METHOD(); }
    Foo(int x) : x(x) { TRACE_METHOD(); }
    ~Foo() { TRACE_METHOD(); };
};

int main(int argc, char * argv[]) 
{
    minipool<Foo> mp(256);

    Foo * p1 = mp.alloc();
    Foo * p2 = mp.alloc(44);

    std::cout << "p1->x=" << p1->x << "\n";
    std::cout << "p2->x=" << p2->x << "\n";

    mp.free(p1);
    mp.free(p2);
}