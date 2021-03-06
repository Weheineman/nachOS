/// Data structures to keep track of executing user programs (address
/// spaces).
///
/// For now, we do not keep any information about address spaces.  The user
/// level CPU state is saved and restored in the thread executing the user
/// program (see `thread.hh`).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_USERPROG_ADDRESSSPACE__HH
#define NACHOS_USERPROG_ADDRESSSPACE__HH


#include "filesys/file_system.hh"
#include "machine/translation_entry.hh"
#include "bin/noff.h"
#include "userprog/syscall.h"
#include "filesys/open_file.hh"


const unsigned USER_STACK_SIZE = 1024;  ///< Increase this as necessary!


class AddressSpace {
public:

    /// Create an address space, initializing it with the program stored in
    /// the file `executable`.
    ///
    /// * `executable` is the open file that corresponds to the program.
    AddressSpace(OpenFile *executable, SpaceId spaceId_);

    /// De-allocate an address space.
    ~AddressSpace();

    /// Initialize user-level CPU registers, before jumping to user code.
    void InitRegisters();

    /// Save/restore address space-specific info on a context switch.
    void SaveState();
    void RestoreState();

    // Returns the virtual page number of a given virtual address, or -1 if
    // the address is invalid.
    int FindContainingPageIndex(int vAddr);

    // Returns true if the page was never loaded to memory, and false otherwise.
    // This means that a pageIndex of a page that is currently stored in the
    // swap file would cause NotLoadedPage to return false.
    bool NotLoadedPage(unsigned pageIndex);

    // Loads a page to the main memory, iff it isn't already loaded.
    // (if it is, it does nothing).
    void LoadPage(unsigned pageIndex);

    // Copies the information from the pageTable at index pageIndex to destPage.
    void CopyPageContent(unsigned pageIndex, TranslationEntry* destPage);

    // Returns the frame index of a given virtual page that is loaded in memory.
    // If the page corresponding to pageIndex is not loaded, the function
    // returns -1.
    int GetPhysicalPage(unsigned int pageIndex);
    
    // Sets the use and dirty flags in the pageTable entry referenced by pageIndex.
    void SetPageFlags(unsigned pageIndex, bool use, bool dirty);

    #ifdef DEMAND_LOADING
        // Stores the page in the swap file.
        void SwapPage(unsigned pageIndex);
    #endif

private:

    // pageTable[i].virtualPage = numPages means the page i has never
    // been loaded.
    // pageTable[i].virtualPage = numPages + 1 means the page i is
    // currently in the swap file.
    TranslationEntry *pageTable;

    /// Number of pages in the virtual address space.
    unsigned numPages;

    /// Structure containing program segment information.
    noffHeader ourNoffHeader;

    /// Pointer to the executable file corresponding to this AddressSpace.
    OpenFile *ourExecutable;

    // User space to which this Address Space belongs.
    SpaceId spaceId;

    #ifdef DEMAND_LOADING
        // Swap file information.
        char *swapFileName;
        OpenFile *swapFile;
    #endif

    /// Loads a page that was never loaded to memory before, to memory.
    void LoadPageFirst(unsigned pageIndex, int physIndex);

    /// Loads a page that is currently in the swap file to memory.
    void LoadPageSwap(unsigned pageIndex, int physIndex);
};


#endif
