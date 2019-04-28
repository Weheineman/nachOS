#ifndef NACHOS_SYNCHCONSOLE
#define NACHOS_SYNCHCONSOLE


#include "threads/synch.hh"
#include "machine/console.hh"


class SynchConsole {
public:

    /// Initialize a synchronous console.
    SynchConsole(const char *readFile, const char *writeFile);

    /// De-allocate the synch console data.
    ~SynchConsole();

    /// External interface -- Nachos kernel code can call these.

    /// Write `ch` to the console display, and return immediately.
    /// `writeHandler` is called when the I/O completes.
    void PutChar(char ch);

    /// Poll the console input.  If a char is available, return it.
    /// Otherwise, return EOF.  `readHandler` is called whenever there is a
    /// char to be gotten.
    char GetChar();


    void WriteDone();
    void ReadAvail();


private:
    Lock* readerLock;
    Lock* writerLock;
    Semaphore* readerSem;
    Semaphore* writerSem;
    Console* console;
};


#endif
