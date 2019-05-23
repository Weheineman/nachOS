CoreMap::CoreMap(){
    nextNisman = 0;
    pageMap = new Bitmap(NUM_PHYS_PAGES);
    ownerAddSpc = new AddressSpace* [NUM_PHYS_PAGES];
    virtualPageNum = new unsigned int [NUM_PHYS_PAGES];
}

CoreMap::~CoreMap(){
    delete pageMap;
    delete [] ownerAddSpc;
    delete [] virtualPageNum;
}

unsigned int
CoreMap::ReservePage(unsigned int virtualPage){
    int index = pageMap -> Find();
    if(index == -1){
        // GUIDIOS: Nismanear
        // ownerAddSpc[nextNisman] -> SwapPage(virtualPageNum[nextNisman]);
        // si es una pagina del proceso actual, hay que anular su entrada
        // en la tlb (si existe).
        index = nextNisman;
        nextNisman = (nextNisman + 1)%NUM_PHYS_PAGES;
    }

    ownerAddSpc[index] = currentThread -> GetAddressSpace();
    virtualPageNum[index] = virtualPage;

    return index;
}
