#ifdef DEMAND_LOADING

#ifndef NACHOS_VMEM_COREMAP_HH
#define NACHOS_VMEM_COREMAP_HH

#include "machine/mmu.hh"
#include "lib/bitmap.hh"
#include "threads/thread.hh"

class CoreMap{
private:
    Bitmap  *pageMap;
    AddressSpace  **ownerAddSp;
    unsigned int *virtualPageNum;
    // GUIDIOS:Plus possibly other flags, such as whether the page is
    // currently locked in memory for I/O purposes.

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

    unsigned int ReservePage(unsigned int virtualPage);

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
