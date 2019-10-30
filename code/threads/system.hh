/// All global variables used in Nachos are defined here.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2018 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_THREADS_SYSTEM__HH
#define NACHOS_THREADS_SYSTEM__HH


#include "thread.hh"
#include "scheduler.hh"
#include "lib/utility.hh"
#include "machine/interrupt.hh"
#include "machine/statistics.hh"
#include "machine/timer.hh"
#include "userprog/synch_console.hh"


/// Initialization and cleanup routines.

// Initialization, called before anything else.
extern void Initialize(int argc, char **argv);

// Cleanup, called when Nachos is done.
extern void Cleanup();


extern Thread *currentThread;        ///< The thread holding the CPU.
extern Thread *threadToBeDestroyed;  ///< The thread that just finished.
extern Scheduler *scheduler;         ///< The ready list.
extern Interrupt *interrupt;         ///< Interrupt status.
extern Statistics *stats;            ///< Performance metrics.
extern Timer *timer;                 ///< The hardware alarm clock.

#ifdef USER_PROGRAM
#include "machine/machine.hh"
#include "lib/bitmap.hh"
extern Machine *machine;  // User program memory and registers.
extern SynchConsole *synchConsole; // Console used in syscall testing
extern Table<Thread*> *threadTable; //Table used for deferencing SpaceIds
extern Bitmap *pageMap;
#ifdef VMEM
    #ifdef DEMAND_LOADING
    #include "vmem/coremap.hh"
    extern CoreMap *coreMap;
    extern unsigned int swapCount;
    #endif
#include "vmem/tlb_handler.hh"
extern TLB_Handler *tlb_handler;
#endif

#endif

#ifdef FILESYS_NEEDED  // *FILESYS* or *FILESYS_STUB*.
#include "filesys/file_system.hh"
extern FileSystem *fileSystem;
#endif

#ifdef FILESYS
#include "filesys/synch_disk.hh"
#include "filesys/direcory_lock_manager.hh"
extern SynchDisk *synchDisk;
extern DirectoryLockManager *directoryLockManager;
#endif

#ifdef NETWORK
#include "network/post.hh"
extern PostOffice *postOffice;
#endif



#endif
