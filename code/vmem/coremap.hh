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

    // GUIDIOS: Nismanear y usar algo mejor el proximo ejercicio :)
    unsigned int nextNisman;

public:
    CoreMap();

    ~CoreMap();

    unsigned int ReservePage(unsigned int virtualPage);
    
    void ReleasePages(AddressSpace* currentSpace);
};

#endif

#endif
