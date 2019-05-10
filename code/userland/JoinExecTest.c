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

int
main(void)
{
    Join(Exec("../userland/AuxTest", 0));

    OpenFileId o = Open("../userland/test.txt");
    char aux[64];
    int len = Read(aux, 64, o);
    Write(aux, len, CONSOLE_OUTPUT);
    Write("\n", 1, CONSOLE_OUTPUT);



    // Hopefully reached.
    Write("This should be printed.\n", 28, CONSOLE_OUTPUT);
    Halt();
}
