TLB_Handler::TLB_Handler(){
    replaceIndex = 0;    
}

TLB_Handler::~TLB_Handler()
{}

TranslationEntry* findEntryToReplace(){
    unsigned i;
    TranslationEntry *tlbRef = machine -> GetMMU() -> tlb;
    for(i = 0; i < TLB_SIZE; i++)
        if(!tlbRef[i].valid)
            return &(tlbRef[i]);    
        
    unsigned returnIndex = replaceIndex;
    replaceIndex = (replaceIndex + 1) % TLB_SIZE;
    return &(tlbRef[returnIndex]);    
}
