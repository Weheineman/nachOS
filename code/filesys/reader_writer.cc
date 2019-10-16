#include "reader_writer.hh"

ReaderWriter::ReaderWriter(){
    ReadCounterLock = new Lock("ReadCounterLock");
    NoReaders = new Condition("ReaderWriter CondVar", ReadCounterLock);
    readCounter = 0;
}

ReaderWriter::~ReaderWriter(){
    delete NoReaders;
    delete ReadCounterLock;
}

void
ReaderWriter::AcquireRead(){
    if(not ReadCounterLock -> IsHeldByCurrentThread()){
        ReadCounterLock -> Acquire();

        readCounter++;

        ReadCounterLock -> Release();
    }
}

void
ReaderWriter::ReleaseRead(){
    if(not ReadCounterLock -> IsHeldByCurrentThread()){
        ReadCounterLock -> Acquire();

        readCounter--;

        if(readCounter == 0)
            NoReaders -> Broadcast();

        ReadCounterLock -> Release();
    }
}

void
ReaderWriter::AcquireWrite(){
    ReadCounterLock -> Acquire();

    while(readCounter > 0)
        NoReaders -> Wait();

}

void
ReaderWriter::ReleaseWrite(){
    NoReaders -> Signal();
    ReadCounterLock -> Release();
}
