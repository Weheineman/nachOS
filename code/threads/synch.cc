/// Routines for synchronizing threads.
///
/// Three kinds of synchronization routines are defined here: semaphores,
/// locks and condition variables (the implementation of the last two are
/// left to the reader).
///
/// Any implementation of a synchronization routine needs some primitive
/// atomic operation.  We assume Nachos is running on a uniprocessor, and
/// thus atomicity can be provided by turning off interrupts.  While
/// interrupts are disabled, no context switch can occur, and thus the
/// current thread is guaranteed to hold the CPU throughout, until interrupts
/// are reenabled.
///
/// Because some of these routines might be called with interrupts already
/// disabled (`Semaphore::V` for one), instead of turning on interrupts at
/// the end of the atomic operation, we always simply re-set the interrupt
/// state back to its original value (whether that be disabled or enabled).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2018 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "synch.hh"
#include "system.hh"


/// Initialize a semaphore, so that it can be used for synchronization.
///
/// * `debugName` is an arbitrary name, useful for debugging.
/// * `initialValue` is the initial value of the semaphore.
Semaphore::Semaphore(const char *debugName, int initialValue)
{
    name  = new char [64];
    strcpy(name, debugName);
    value = initialValue;
    queue = new List<Thread *>;
}

/// De-allocate semaphore, when no longer needed.
///
/// Assume no one is still waiting on the semaphore!
Semaphore::~Semaphore()
{
    delete queue;
    delete [] name;
}

const char *
Semaphore::GetName() const
{
    return name;
}

/// Wait until semaphore `value > 0`, then decrement.
///
/// Checking the value and decrementing must be done atomically, so we need
/// to disable interrupts before checking the value.
///
/// Note that `Thread::Sleep` assumes that interrupts are disabled when it is
/// called.
void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(INT_OFF);
      // Disable interrupts.

    while (value == 0) {  // Semaphore not available.
        queue->Append(currentThread);  // So go to sleep.
        currentThread->Sleep();
    }
    value--;  // Semaphore available, consume its value.

    // Print debug message
    char acquireMsg[64];
    snprintf(acquireMsg, 64, "%s%s%s%s%c", "P() called on ", GetName(),
        " by ", currentThread->GetName(), '\n');
    DEBUG('s', acquireMsg);

    interrupt->SetLevel(oldLevel);  // Re-enable interrupts.
}

/// Increment semaphore value, waking up a waiter if necessary.
///
/// As with `P`, this operation must be atomic, so we need to disable
/// interrupts.  `Scheduler::ReadyToRun` assumes that threads are disabled
/// when it is called.
void
Semaphore::V()
{
    IntStatus oldLevel = interrupt->SetLevel(INT_OFF);

    Thread *thread = queue->Pop();
    if (thread != nullptr)
        // Make thread ready, consuming the `V` immediately.
        scheduler->ReadyToRun(thread);
    value++;

    // Print debug message
    char releaseMsg[64];
    snprintf(releaseMsg, 64, "%s%s%s%s%c", "V() called on ", GetName(),
        " by ", currentThread->GetName(), '\n');
    DEBUG('s', releaseMsg);

    interrupt->SetLevel(oldLevel);
}

/// Dummy functions -- so we can compile our later assignments.
///
/// Note -- without a correct implementation of `Condition::Wait`, the test
/// case in the network assignment will not work!

Lock::Lock(const char *debugName)
{
    name = debugName;
    semaphoreName = new char [64];
    strcpy(semaphoreName, "Semaphore of ");
    strcat(semaphoreName, debugName);
    lockSemaphore = new Semaphore(semaphoreName, 1);
    lockOwner = nullptr;
}

Lock::~Lock()
{
    delete lockSemaphore;
    delete [] semaphoreName;
}

const char *
Lock::GetName() const
{
    return name;
}

void
Lock::Acquire()
{
    ASSERT(not IsHeldByCurrentThread());

    // Prevent priority inversion. This happens if the lock is taken by a
    // thread with lower priority.
    if(lockOwner != nullptr)
        if(currentThread->GetPriority() > lockOwner->GetPriority())
            scheduler->PromoteThread(lockOwner, currentThread->GetPriority());

    lockSemaphore->P();
    lockOwner = currentThread;
}

void
Lock::Release()
{
    ASSERT(IsHeldByCurrentThread());

    // Restore the original thread priority, in case it had been promoted.
    scheduler->DemoteThread(currentThread);

    lockOwner = nullptr;
    lockSemaphore->V();
}

bool
Lock::IsHeldByCurrentThread() const
{
    return lockOwner == currentThread;
}

Thread*
Lock::LockOwner()
{
    return lockOwner;
}

Condition::Condition(const char *debugName, Lock *conditionLock_)
{
    name = debugName;
    conditionLock = conditionLock_;

    queueLockName = new char [64];
    strcpy(queueLockName, "Queue lock of ");
    strcat(queueLockName, debugName);
    queueLock = new Lock(queueLockName);

    sleepQueueName = new char [64];
    strcpy(sleepQueueName, "Sleep queue of ");
    strcat(sleepQueueName, debugName);
    sleepQueue = new Semaphore(sleepQueueName, 0);

    handshakeSemaphoreName = new char [64];
    strcpy(handshakeSemaphoreName, "Handshake semaphore of ");
    strcat(handshakeSemaphoreName, debugName);
    handshakeSemaphore = new Semaphore(handshakeSemaphoreName, 0);

    sleeperAmount = 0;
}

Condition::~Condition()
{
    delete queueLock;
    delete [] queueLockName;
    delete sleepQueue;
    delete [] sleepQueueName;
    delete handshakeSemaphore;
    delete [] handshakeSemaphoreName;
}

const char *
Condition::GetName() const
{
    return name;
}

void
Condition::Wait()
{
    ASSERT(conditionLock->IsHeldByCurrentThread());

    queueLock->Acquire();
    sleeperAmount++;
    queueLock->Release();

    conditionLock->Release();
    sleepQueue->P();

    // When woken up, reacquire lock
    conditionLock->Acquire();
    handshakeSemaphore->V();
}

void
Condition::Signal()
{
    ASSERT(conditionLock->IsHeldByCurrentThread());
    queueLock->Acquire();
    if(sleeperAmount > 0){
        sleeperAmount--;
        sleepQueue->V();
        handshakeSemaphore->P();
    }
    queueLock->Release();
}

void
Condition::Broadcast()
{
    ASSERT(conditionLock->IsHeldByCurrentThread());
    queueLock->Acquire();
    while(sleeperAmount > 0){
        sleeperAmount--;
        sleepQueue->V();
        handshakeSemaphore->P();
    }
    queueLock->Release();
}

Port::Port(const char* debugName)
{
    name = debugName;
    emptyBuffer = true;

    portLockName = new char [64];
    strcpy(portLockName, "Buffer lock of ");
    strcat(portLockName, debugName);
    portLock = new Lock(portLockName);

    senderName = new char [64];
    strcpy(senderName, "Sender of ");
    strcat(senderName, debugName);
    sender = new Condition(senderName, portLock);

    receiverName = new char [64];
    strcpy(receiverName, "Receiver of ");
    strcat(receiverName, debugName);
    receiver = new Condition(receiverName, portLock);
}

Port::~Port()
{
    delete portLock;
    delete [] portLockName;
    delete sender;
    delete [] senderName;
    delete receiver;
    delete [] receiverName;
}

const char*
Port::GetName() const
{
    return name;
}

void
Port::Send(int message)
{
    portLock->Acquire();

    while(not emptyBuffer)
        sender->Wait();

    messageBuffer = message;
    emptyBuffer = false;
    receiver->Signal();


    portLock->Release();
}

void
Port::Receive(int *message)
{
    portLock->Acquire();

    while(emptyBuffer)
        receiver->Wait();

    *message = messageBuffer;
    emptyBuffer = true;
    sender->Signal();

    portLock->Release();
}
