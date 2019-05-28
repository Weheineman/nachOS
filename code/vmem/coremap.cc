#ifdef DEMAND_LOADING

#include "coremap.hh"
#include "threads/system.hh"

CoreMap::CoreMap(){
    #ifdef LRU
        idleCounter = new unsigned int [NUM_PHYS_PAGES];
    #else
        nextRemoved = 0;
    #endif
    pageMap = new Bitmap(NUM_PHYS_PAGES);
    ownerAddSp = new AddressSpace* [NUM_PHYS_PAGES];
    virtualPageNum = new unsigned int [NUM_PHYS_PAGES];
}

CoreMap::~CoreMap(){
    #ifdef LRU
        delete [] idleCounter;
    #endif

    delete pageMap;
    delete [] ownerAddSp;
    delete [] virtualPageNum;
}

unsigned int
CoreMap::ReservePage(unsigned int virtualPage){
    int index = pageMap -> Find();

    // If the pageMap is full
    if(index == -1){
        #ifdef LRU
            // LRU.
            index = FindLRU();
        #else
            // FIFO.
            index = nextRemoved;
            nextRemoved = (nextRemoved + 1)%NUM_PHYS_PAGES;
        #endif

        // Send the currently loaded page to the swap file.
        ownerAddSp[index] -> SwapPage(virtualPageNum[index]);
    }

    ownerAddSp[index] = currentThread -> GetAddressSpace();
    virtualPageNum[index] = virtualPage;

    return index;
}

void
CoreMap::ReleasePages(AddressSpace* currentSpace){
    for(unsigned i = 0; i < NUM_PHYS_PAGES; i++)
        if(ownerAddSp[i] == currentSpace)
            pageMap -> Clear(i);
}

#ifdef LRU

void
CoreMap::UpdateIdleCounter(unsigned int loadedIndex){
    for(unsigned int ind = 0; ind < NUM_PHYS_PAGES; ind++)
        if(ind == loadedIndex)
            idleCounter[ind] = 0;
        else
            idleCounter[ind]++;
}

unsigned int
CoreMap::FindLRU(){
    unsigned int lru = 0;
    unsigned int maxIdle = 0;
    for(unsigned int ind = 0; ind < NUM_PHYS_PAGES; ind++)
        if(idleCounter[ind] > maxIdle){
            lru = ind;
            maxIdle = idleCounter[ind];
        }

    return lru;
}

#endif

#endif
