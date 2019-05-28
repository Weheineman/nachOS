#include "tlb_handler.hh"
#include "threads/system.hh"


TLB_Handler::TLB_Handler(){
    replaceIndex = 0;
}

TLB_Handler::~TLB_Handler()
{}

TranslationEntry *
TLB_Handler::FindEntryToReplace(){
    unsigned i;
    TranslationEntry *tlbRef = machine -> GetMMU() -> tlb;
    for(i = 0; i < TLB_SIZE; i++)
        if(!tlbRef[i].valid)
            return &(tlbRef[i]);

    unsigned returnIndex = replaceIndex;
    replaceIndex = (replaceIndex + 1) % TLB_SIZE;
    return &(tlbRef[returnIndex]);
}

void
TLB_Handler::ReplaceTLBEntry(unsigned newPageIndex){
	TranslationEntry *oldPage = FindEntryToReplace();

    // GUIDIOS: Cuando sacamos una entrada valida, hay que actualizar los campos
    // de la pageTable.

    AddressSpace *currentSpace = currentThread -> GetAddressSpace();

    #ifdef LRU
        int physIndex = currentSpace -> GetPhysicalPage(newPageIndex);

        // Check that GetPhysicalPage returned successfully.
        ASSERT(physIndex >= 0);

        // Update the physical page usage table.
        coreMap -> UpdateIdleCounter(physIndex);
    #endif

	currentSpace -> CopyPageContent(newPageIndex, oldPage);
}
