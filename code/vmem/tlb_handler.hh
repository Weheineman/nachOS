#ifndef NACHOS_VMEM_TLBHANDLER_HH
#define NACHOS_VMEM_TLBHANDLER_HH

#include "machine/mmu.hh"

class TLB_Handler {
public:
    TLB_Handler();
    ~TLB_Handler();
    
    void ReplaceTLBEntry(unsigned newPageIndex);

private:
    unsigned replaceIndex;
    
    TranslationEntry* FindEntryToReplace();
};



#endif
