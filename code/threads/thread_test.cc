/// Simple test case for the threads assignment.
///
/// Create several threads, and have them context switch back and forth
/// between themselves by calling `Thread::Yield`, to illustrate the inner
/// workings of the thread system.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "system.hh"
#include "synch.hh"
#include <utility>

// String Semaphore Pair, used to test Semaphores.
typedef std::pair<char*, Semaphore*> StrSemPair;


/// Loop 10 times, yielding the CPU to another ready thread each iteration.
///
/// * `name` points to a string with a thread name, just for debugging
///   purposes.
void
SimpleThread(void *name_)
{
    // Reinterpret arg `name` as a string.
    char *name = (char *) name_;

    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.
    for (unsigned num = 0; num < 10; num++) {
        printf("*** Thread `%s` is running: iteration %u\n", name, num);
        currentThread->Yield();
    }
    printf("!!! Thread `%s` has finished\n", name);
}


// Same as SimpleThread but is called only if SEMAPHORE_TEST is defined.
// Uses the semaphore declared in ThreadTest.
void SemaphoreThread(void *pointerPair_){
    // Reinterpret arg `pointerPair` as a pair<*char, *Semaphore> pointer.
    StrSemPair *pointerPair = (StrSemPair*) pointerPair_;

    // Rename the elements of pointerPair for more clarity
    char *name = pointerPair->first;
    Semaphore *testSemaphore = pointerPair->second;

    testSemaphore->P();

    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.
    for (unsigned num = 0; num < 10; num++) {
        printf("*** Thread `%s` is running: iteration %u\n", name, num);
        currentThread->Yield();
    }
    printf("!!! Thread `%s` has finished\n", name);

    testSemaphore->V();
}


/// Set up a ping-pong between several threads.
///
/// Do it by launching ten threads which call `SimpleThread`, and finally
/// calling `SimpleThread` ourselves.
void
ThreadTest()
{
    DEBUG('t', "Entering thread test\n");
    // Amount of threads to launch
    const int threadAmount = 5;

    #ifdef SEMAPHORE_TEST
    // Initial value of the semaphore
    const int semInit = 3;
    Semaphore *testSemaphore = new Semaphore("Ejercicio 15", semInit);
    #endif

    // name[i] will contain the name of the (i+1)th process. This happens
    // because name is 0-indexed and the processes are 1-indexed.
    char **name = new char* [threadAmount];
    for(int threadNum = 1; threadNum <= threadAmount; threadNum++){
        char *currentName = name[threadNum-1] = new char [64];
        snprintf(currentName, 64, "%s%d", "Number ", threadNum);
        Thread *newThread = new Thread(currentName);
        #ifdef SEMAPHORE_TEST
        // Launch semaphore test threads
        StrSemPair *pointerPair = new StrSemPair;
        *pointerPair = std::make_pair(currentName, testSemaphore);
        newThread->Fork(SemaphoreThread, (void*) pointerPair);
        #else
        // Launch simple threads
        newThread->Fork(SimpleThread, (void*) currentName);
        #endif
    }
}
