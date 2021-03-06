/// Data structures for the thread dispatcher and scheduler.
///
/// Primarily, the list of threads that are ready to run.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2018 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_THREADS_SCHEDULER__HH
#define NACHOS_THREADS_SCHEDULER__HH


#include "thread.hh"
#include "lib/list.hh"


/// The following class defines the scheduler/dispatcher abstraction --
/// the data structures and operations needed to keep track of which
/// thread is running, and which threads are ready but not running.
class Scheduler {
public:

    /// Initialize list of ready threads.
    Scheduler();

    /// De-allocate ready list.
    ~Scheduler();

    /// Thread can be dispatched.
    void ReadyToRun(Thread *thread);

    /// Dequeue first thread on the ready list, if any, and return thread.
    Thread *FindNextToRun();

    /// Cause `nextThread` to start running.
    void Run(Thread *nextThread);

    // Print contents of ready list.
    void Print();

    // Increases the priority of promoted and puts it in the corresponding
    // queue
    void PromoteThread(Thread *promoted, int newPriority);

    // Returns the maximum level of priority a thread can have
    const int GetPriorityAmount() const;

private:
    // Amount of levels of priority that can be assigned to threads.
    static const int priorityAmount = 10;

    // Queue of threads that are ready to run, but not running.
    List<Thread*> *readyList[priorityAmount];

};


#endif
