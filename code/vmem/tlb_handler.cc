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

    AddressSpace *currentSpace = currentThread -> GetAddressSpace();

	// If the page that is going to be removed is valid (so it maps a frame from the current 
	// process), then copy its flags to the corresponding pageTable entry.
	if(oldPage->valid)
		currentSpace -> SetPageFlags(oldPage -> virtualPage,
		                             oldPage -> use,
		                             oldPage -> dirty);
		                             
    #ifdef LRU
        int physIndex = currentSpace -> GetPhysicalPage(newPageIndex);

        // Check that GetPhysicalPage returned successfully.
        ASSERT(physIndex >= 0);

        // Update the physical page usage table.
        coreMap -> UpdateIdleCounter(physIndex);
    #endif

	// Copy the content from the new page into the corresponding TLB entry.
	currentSpace -> CopyPageContent(newPageIndex, oldPage);
}
