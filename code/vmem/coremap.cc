#ifdef DEMAND_LOADING

#include "coremap.hh"
#include "threads/system.hh"

CoreMap::CoreMap(){
    nextNisman = 0;
    pageMap = new Bitmap(NUM_PHYS_PAGES);
    ownerAddSp = new AddressSpace* [NUM_PHYS_PAGES];
    virtualPageNum = new unsigned int [NUM_PHYS_PAGES];
}

CoreMap::~CoreMap(){
    delete pageMap;
    delete [] ownerAddSp;
    delete [] virtualPageNum;
}

unsigned int
CoreMap::ReservePage(unsigned int virtualPage){
    int index = pageMap -> Find();

    // If the pageMap is full
    if(index == -1){
        ownerAddSp[nextNisman] -> SwapPage(virtualPageNum[nextNisman]);
        index = nextNisman;
        nextNisman = (nextNisman + 1)%NUM_PHYS_PAGES;
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

#endif
