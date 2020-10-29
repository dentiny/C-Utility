#include <new>
#include <cstdlib>
#include <iostream>
#define ARRAY_SIZE 10

class MyClass
{
public:
    MyClass() : 
        val { new int(10) } 
    { 
        std::cout << "ctor called" << std::endl; 
    }
    ~MyClass() 
    {
        delete(val); 
        std::cout << "dtor called" << std::endl; 
    }

private:
    int * val;
};

int main()
{
    // allocate memory
    MyClass * mem = static_cast<MyClass*>(malloc(sizeof(MyClass) * ARRAY_SIZE));

    // placement new
    for(int idx = 0; idx < ARRAY_SIZE; ++idx) 
    {
        new (mem + idx) MyClass;
    }

    // call destructor
    for(int idx = 0; idx < ARRAY_SIZE; ++idx)
    {
        MyClass * ptr = mem + idx;
        ptr->~MyClass();
    }

    // deallocate memory
    free(mem);
}