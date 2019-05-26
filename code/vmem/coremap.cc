#ifdef DEMAND_LOADING

#include "coremap.hh"
#include "threads/system.hh"

CoreMap::CoreMap(){
    nextNisman = 0;
    pageMap = new Bitmap(NUM_PHYS_PAGES);
    ownerThread = new Thread* [NUM_PHYS_PAGES];
    virtualPageNum = new unsigned int [NUM_PHYS_PAGES];
}

CoreMap::~CoreMap(){
    delete pageMap;
    delete [] ownerThread;
    delete [] virtualPageNum;
}

unsigned int
CoreMap::ReservePage(unsigned int virtualPage){
    int index = pageMap -> Find();

    // If the pageMap is full
    if(index == -1){
        ownerThread[nextNisman] -> SwapPage(virtualPageNum[nextNisman]);
        index = nextNisman;
        nextNisman = (nextNisman + 1)%NUM_PHYS_PAGES;
    }

    ownerThread[index] = currentThread;
    virtualPageNum[index] = virtualPage;

    return index;
}

#endif
