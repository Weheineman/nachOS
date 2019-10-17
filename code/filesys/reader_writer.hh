#ifndef FILESYS_READERWRITER__HH
#define FILESYS_READERWRITER__HH

#include "threads/synch.hh"

class Lock;
class Condition;

class ReaderWriter {
public:
    ReaderWriter();
    ~ReaderWriter();
    void AcquireRead();
    void ReleaseRead();
    void AcquireWrite();
    void ReleaseWrite();
private:
    Lock *RWLock, *ReadCounterLock;
    Condition *NoReaders;
    int readCounter;
};

#endif
