#ifdef DEMAND_LOADING

#ifndef NACHOS_VMEM_COREMAP_HH
#define NACHOS_VMEM_COREMAP_HH

#include "machine/mmu.hh"
#include "lib/bitmap.hh"
#include "threads/thread.hh"

class CoreMap{
private:
    // Used to manage availability of the physical pages.
    Bitmap  *pageMap;

    // Information on the assigned physical pages.
    AddressSpace  **ownerAddSp;
    unsigned int *virtualPageNum;

    #ifdef LRU
        // idleCounter[i] counts the amount times there was a memory access not
        // involving the physical page i. The last accessed page has value 0.
        unsigned int *idleCounter;
    #else
        // Used for FIFO.
        unsigned int nextRemoved;
    #endif

public:
    CoreMap();

    ~CoreMap();

    // Reserves a physical page and returns the index. If all pages are already
    // assigned, it chooses one to send to the swap file. The method used for
    // this is LRU if the LRU compilation flag is defined. Otherwise, it uses
    // FIFO.
    unsigned int ReservePage(unsigned int virtualPage);

    // Makes all previously reserved pages of a given Address Space available.
    void ReleasePages(AddressSpace* currentSpace);

    #ifdef LRU
        // Sets idleCounter to 0 at the given index and increases the rest by 1.
        void UpdateIdleCounter(unsigned int loadedIndex);

        // Finds the index with the highest idleCounter.
        unsigned int FindLRU();
    #endif
};

#endif

#endif
