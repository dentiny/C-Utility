#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

static ucontext_t uctx_main, uctx_func1, uctx_func2;

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

static void func1(void)
{
    printf("func1: started\n");
    printf("func1: swapcontext(&uctx_func1, &uctx_func2)\n");
    if (swapcontext(&uctx_func1, &uctx_func2) == -1) // switch into uctx_func2
    {
        handle_error("swapcontext");
    }
    printf("func1: returning\n");
}

static void func2(void)
{
    printf("func2: started\n");
    printf("func2: swapcontext(&uctx_func2, &uctx_func1)\n");
    if (swapcontext(&uctx_func2, &uctx_func1) == -1) // switch into uctx_func1
    {   
        handle_error("swapcontext");
    }
    printf("func2: returning\n");
}

int main(int argc, char * argv[])
{
    char func1_stack[16384];
    char func2_stack[16384];

    // set up coroutine uctx_func1
    if(getcontext(&uctx_func1) == -1)
    {   
        handle_error("getcontext");
    }
    uctx_func1.uc_stack.ss_sp = func1_stack;
    uctx_func1.uc_stack.ss_size = sizeof(func1_stack);
    uctx_func1.uc_link = &uctx_main; // after executing uctx_func1, execute uctx_main
    makecontext(&uctx_func1, func1, 0); // entry function is func1

    // set up coroutine uctx_func2
    if(getcontext(&uctx_func2) == -1)
    {   
        handle_error("getcontext");
    }
    uctx_func2.uc_stack.ss_sp = func2_stack;
    uctx_func2.uc_stack.ss_size = sizeof(func2_stack);
    uctx_func2.uc_link = (argc > 1) ? NULL : &uctx_func1; // after executing uctx_func2, execute uctx_func2
    makecontext(&uctx_func2, func2, 0); // entry function is func2

    // begin switch into coroutine
    printf("main: swapcontext(&uctx_main, &uctx_func2)\n");
    if(swapcontext(&uctx_main, &uctx_func2) == -1) // switch into uctx_func2
    {    
        handle_error("swapcontext");
    }
    printf("main: exiting\n");

    exit(EXIT_SUCCESS);
}

/*
with only one argument:
main: swapcontext(&uctx_main, &uctx_func2)
func2: started
func2: swapcontext(&uctx_func2, &uctx_func1)
func1: started
func1: swapcontext(&uctx_func1, &uctx_func2)
func2: returning
func1: returning
main: exiting

with more-than-one argument:
main: swapcontext(&uctx_main, &uctx_func2)
func2: started
func2: swapcontext(&uctx_func2, &uctx_func1)
func1: started
func1: swapcontext(&uctx_func1, &uctx_func2)
func2: returning
func1: returning
main: exiting
*/