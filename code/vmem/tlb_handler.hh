#ifndef NACHOS_VMEM_TLBHANDLER_HH
#define NACHOS_VMEM_TLBHANDLER_HH

#include "machine/mmu.hh"

class TLB_Handler {
public:
    TLB_Handler();
    ~TLB_Handler();
    TranslationEntry* findEntryToReplace();

private:
    unsigned replaceIndex;
}



#endif
