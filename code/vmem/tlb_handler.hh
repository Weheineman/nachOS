#ifndef NACHOS_VMEM_TLBHANDLER_HH
#define NACHOS_VMEM_TLBHANDLER_HH

#include "machine/mmu.hh"

class TLB_Handler {
public:
    TLB_Handler();
    ~TLB_Handler();
    
    // Given a virtual page index from the current process, replaces a TLB entry
    // with the page entry corresponding to that index.
    void ReplaceTLBEntry(unsigned newPageIndex);

private:
    unsigned replaceIndex;
    
    // Returns a pointer to the entry to be replaced.
    TranslationEntry* FindEntryToReplace();
};



#endif
