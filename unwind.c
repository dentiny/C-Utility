// Print stacktrace with `addr2line`.
// gcc -static -fno-omit-frame-pointer -g3 -O0 unwind.c -o unwind

#include <stdio.h>
#include <stdlib.h>

const char *binary = NULL;

struct frame {
  struct frame *next; // push %rbp
  void *addr;         // call f (pushed retaddr)
};

void backtrace() {
  struct frame *f = NULL;
  char cmd[1024] = { 0 };
  extern char end;

  asm volatile ("movq %%rbp, %0" : "=g"(f));
  for (; f->addr < (void *)&end; f = f->next) {
    printf("%016lx  ", (long)f->addr); fflush(stdout);
    sprintf(cmd, "addr2line -e %s %p", binary, f->addr);
    system(cmd);
  }
}

void bar() {
  backtrace();
}

void foo() {
  bar();
}

int main(int argc, char *argv[]) {
  binary = argv[0];
  foo();
}
