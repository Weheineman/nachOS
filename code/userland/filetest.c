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
    Create("test.txt");
    OpenFileId o = Open("test.txt");
    Write("Look mom, I'm in the file!", 26, o);
    Close(o);
    o = Open("test.txt");
    char aux[64];
    Read(aux, 64, o);
    Write(aux, 64, CONSOLE_OUTPUT);

    // Read(aux, 64, CONSOLE_INPUT);
    // Write("Hello World", 12, CONSOLE_OUTPUT);
    // Write(aux, 64, CONSOLE_OUTPUT);
    Halt();
    // Not reached.
}
