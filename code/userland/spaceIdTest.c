/// Simple program to test whether running a user program works.
///
/// Just do a “syscall” that shuts down the OS.
///
/// NOTE: for some reason, user programs with global data structures
/// sometimes have not worked in the Nachos environment.  So be careful out
/// there!  One option is to allocate data structures as automatics within a
/// procedure, but if you do this, you have to be careful to allocate a big
/// enough stack to hold the automatics!

#include "syscall.h"

void
intToStr(char* p, int n){
    p[0] = '0'+n;
    p[1] = 0;
}

int
main(void)
{
    char p1[10], p2[10];

    intToStr(p1, Exec("../userland/halt"));
    intToStr(p2, Exec("../userland/halt"));

    Write(p1, 1, CONSOLE_OUTPUT);
    Write("\n", 1, CONSOLE_OUTPUT);
    Write(p2, 1, CONSOLE_OUTPUT);
    Write("\n", 1, CONSOLE_OUTPUT);

    // Hopefully reached.
    Write("This should be printed.\n", 28, CONSOLE_OUTPUT);
}
