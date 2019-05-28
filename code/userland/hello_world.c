#include "syscall.h"


int
main(void)
{
    Write("Hello world!\n", 14, CONSOLE_OUTPUT);
    Exit(0);
}
