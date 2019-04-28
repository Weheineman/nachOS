#include "synch_console.hh"

void ReadAvailProxy(void* data){
    ASSERT(data != nullptr);
    ((SynchConsole *) data) -> ReadAvail();
}

void WriteDoneProxy(void* data){
    ASSERT(data != nullptr);
    ((SynchConsole *) data) -> WriteDone();
}


SynchConsole::SynchConsole(const char *readFile, const char *writeFile){
    readerLock = new Lock("Reader Lock");
    writerLock = new Lock("Writer Lock");
    readerSem = new Semaphore("Reader Semaphore", 0);
    writerSem = new Semaphore("Writer Semaphore", 0);
    console = new Console(readFile, writeFile, ReadAvailProxy, WriteDoneProxy, this);
}

SynchConsole::~SynchConsole()
{
    delete readerLock;
    delete writerLock;
    delete readerSem;
    delete writerSem;
    delete console;
}

void SynchConsole::PutChar(char ch){
    writerLock -> Acquire();
    console -> PutChar(ch);
    writerSem -> P();
    writerLock -> Release();
}

char SynchConsole::GetChar(){
    readerLock -> Acquire();
    readerSem -> P();
    char returnValue = console -> GetChar();
    readerLock -> Release();
    return returnValue;
}

void SynchConsole::ReadAvail(){
    readerSem -> V();
}

void SynchConsole::WriteDone(){
    writerSem -> V();
}
