/// Routines to manage address spaces (executing user programs).
///
/// In order to run a user program, you must:
///
/// 1. Link with the `-N -T 0` option.
/// 2. Run `coff2noff` to convert the object file to Nachos format (Nachos
///    object code format is essentially just a simpler version of the UNIX
///    executable object code format).
/// 3. Load the NOFF file into the Nachos file system (if you have not
///    implemented the file system yet, you do not need to do this last
///    step).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "address_space.hh"
#include "machine/endianness.hh"
#include "threads/system.hh"
#include "vmem/coremap.hh"
#include "lib/utility.hh"


/// Do little endian to big endian conversion on the bytes in the object file
/// header, in case the file was generated on a little endian machine, and we
/// are re now running on a big endian machine.
static void
SwapHeader(noffHeader *noffH)
{
    ASSERT(noffH != nullptr);

    noffH->noffMagic              = WordToHost(noffH->noffMagic);
    noffH->code.size              = WordToHost(noffH->code.size);
    noffH->code.virtualAddr       = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr        = WordToHost(noffH->code.inFileAddr);
    noffH->initData.size          = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr   = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr    = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size        = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr =
      WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr  = WordToHost(noffH->uninitData.inFileAddr);
}


uint32_t virtualPageIndex(uint32_t virtualAddress){
	return virtualAddress / PAGE_SIZE;
}

uint32_t virtualPageOffset(uint32_t virtualAddress){
	return virtualAddress % PAGE_SIZE;
}

/// Create an address space to run a user program.
///
/// Load the program from a file `executable`, and set everything up so that
/// we can start executing user instructions.
///
/// Assumes that the object code file is in NOFF format.
///
/// First, set up the translation from program memory to physical memory.
///
/// * `executable` is the file containing the object code to load into
///   memory.
AddressSpace::AddressSpace(OpenFile *executable)
{
    ASSERT(executable != nullptr);

    noffHeader noffH;
    executable->ReadAt((char *) &noffH, sizeof noffH, 0);
    if (noffH.noffMagic != NOFF_MAGIC &&
          WordToHost(noffH.noffMagic) == NOFF_MAGIC)
        SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFF_MAGIC);

    // How big is address space?

    unsigned size = noffH.code.size + noffH.initData.size
                    + noffH.uninitData.size + USER_STACK_SIZE;
      // We need to increase the size to leave room for the stack.
    numPages = DivRoundUp(size, PAGE_SIZE);
    size = numPages * PAGE_SIZE;




    #ifndef DEMAND_LOADING
    // GUIDIOS: Revisar toda la funcion cuando tengamos coremap.
    // Check we are not trying to run anything too big.
    ASSERT(numPages <= pageMap -> CountClear());

    DEBUG('a', "Initializing address space, num pages %u, size %u\n",
          numPages, size);
    #endif

    pageTable = new TranslationEntry[numPages];

    // Store our noffHeader.
    ourNoffHeader = noffH;

    // Store the open file reference.
    ourExecutable = executable;

    #ifndef DEMAND_LOADING

    // First, set up the translation.
    char *mainMemory = machine->GetMMU()->mainMemory;

    for (unsigned i = 0; i < numPages; i++) {
        pageTable[i].virtualPage  = i;
		pageTable[i].physicalPage = pageMap -> Find();
        pageTable[i].valid        = true;
        pageTable[i].use          = false;
        pageTable[i].dirty        = false;
        pageTable[i].readOnly     = false;
          // If the code segment was entirely on a separate page, we could
          // set its pages to be read-only.

        //Zero out the page.
        unsigned pageIndex = pageTable[i].physicalPage;
		memset(mainMemory + pageIndex * PAGE_SIZE, 0, PAGE_SIZE);
    }

    // Then, copy in the code and data segments into memory.

    if (noffH.code.size > 0) {
        uint32_t startingPage = virtualPageIndex(noffH.code.virtualAddr);
        uint32_t startingOffset = virtualPageOffset(noffH.code.virtualAddr);
        uint32_t writeAmount = minn(noffH.code.size, PAGE_SIZE - startingOffset);
        uint32_t targetAddress = pageTable[startingPage].physicalPage * PAGE_SIZE + startingOffset;

		DEBUG('a', "Initializing code segment at 0x%X, physical address 0x%X, size %u\n",
              noffH.code.virtualAddr, targetAddress, writeAmount);

		executable->ReadAt(&(mainMemory[targetAddress]),
		                   writeAmount, noffH.code.inFileAddr);

		uint32_t writtenSize = writeAmount;
		uint32_t writtenPages = 1;
		while(writtenSize < noffH.code.size){
			writeAmount = minn(noffH.code.size - writtenSize, PAGE_SIZE);
			targetAddress = pageTable[startingPage + writtenPages].physicalPage * PAGE_SIZE;

			DEBUG('a', "Initializing code segment at 0x%X, physical address 0x%X, size %u\n",
				  noffH.code.virtualAddr + writtenSize, targetAddress, writeAmount);

			executable->ReadAt(&(mainMemory[targetAddress]),
		                   writeAmount, noffH.code.inFileAddr + writtenSize);

			writtenSize += writeAmount;
			writtenPages++;
		}

    }

    if (noffH.initData.size > 0) {
        uint32_t startingPage = virtualPageIndex(noffH.initData.virtualAddr);
        uint32_t startingOffset = virtualPageOffset(noffH.initData.virtualAddr);
		uint32_t writeAmount = minn(noffH.initData.size, PAGE_SIZE - startingOffset);
        uint32_t targetAddress = pageTable[startingPage].physicalPage * PAGE_SIZE + startingOffset;

		DEBUG('a', "Initializing data segment at 0x%X, physical address 0x%X, size %u\n",
              noffH.initData.virtualAddr, targetAddress, writeAmount);

		executable->ReadAt(&(mainMemory[targetAddress]),
		                   writeAmount, noffH.initData.inFileAddr);

		uint32_t writtenSize = writeAmount;
		uint32_t writtenPages = 1;
		while(writtenSize < noffH.initData.size){
			writeAmount = minn(noffH.initData.size - writtenSize, PAGE_SIZE);
			targetAddress = pageTable[startingPage + writtenPages].physicalPage * PAGE_SIZE;

			DEBUG('a', "Initializing data segment at 0x%X, physical address 0x%X, size %u\n",
				  noffH.initData.virtualAddr + writtenSize, targetAddress, writeAmount);

			executable->ReadAt(&(mainMemory[targetAddress]),
		                   writeAmount, noffH.initData.inFileAddr + writtenSize);

			writtenSize += writeAmount;
			writtenPages++;
		}

	}

    #else

    for (unsigned i = 0; i < numPages; i++) {
        ///Using an invalid value for virtual pages to know when
        /// a page has not yet been loaded.
        pageTable[i].virtualPage  = numPages;
		pageTable[i].physicalPage = pageMap -> Find();
        pageTable[i].valid        = true;
        pageTable[i].use          = false;
        pageTable[i].dirty        = false;
        pageTable[i].readOnly     = false;
    }

    #endif
}

/// Deallocate an address space.
///
/// Nothing for now!
AddressSpace::~AddressSpace()
{
	for(unsigned i = 0; i < numPages; i++)
		pageMap -> Clear(pageTable[i].physicalPage);

    delete [] pageTable;
    delete ourExecutable;
}

/// Set the initial values for the user-level register set.
///
/// We write these directly into the “machine” registers, so that we can
/// immediately jump to user code.  Note that these will be saved/restored
/// into the `currentThread->userRegisters` when this thread is context
/// switched out.
void
AddressSpace::InitRegisters()
{
    for (unsigned i = 0; i < NUM_TOTAL_REGS; i++)
        machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of `Start`.
    machine->WriteRegister(PC_REG, 0);

    // Need to also tell MIPS where next instruction is, because of branch
    // delay possibility.
    machine->WriteRegister(NEXT_PC_REG, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we do not
    // accidentally reference off the end!
    machine->WriteRegister(STACK_REG, numPages * PAGE_SIZE - 16);
    DEBUG('a', "Initializing stack register to %u\n",
          numPages * PAGE_SIZE - 16);
}

/// On a context switch, save any machine state, specific to this address
/// space, that needs saving.
void
AddressSpace::SaveState()
{
    #ifdef USE_TLB
    TranslationEntry *tlbRef = machine -> GetMMU() -> tlb;
	for(unsigned i = 0; i < TLB_SIZE; i++){
        if(tlbRef[i].valid){
            unsigned pageIndex = tlbRef[i].virtualPage;
            pageTable[pageIndex].use = tlbRef[i].use;
            pageTable[pageIndex].dirty = tlbRef[i].dirty;
        }
    }
    #endif
}

/// On a context switch, restore the machine state so that this address space
/// can run.
///
/// For now, tell the machine where to find the page table.
void
AddressSpace::RestoreState()
{
	#ifdef USE_TLB

    TranslationEntry *tlbRef = machine -> GetMMU() -> tlb;
	for(unsigned i = 0; i < TLB_SIZE; i++)
		tlbRef[i].valid = false;

    #else

    machine->GetMMU()->pageTable     = pageTable;
    machine->GetMMU()->pageTableSize = numPages;

	#endif
}


int
AddressSpace::FindContainingPageIndex (int vAddr)
{
    if(vAddr < 0)
        return -1;

    int index = vAddr / PAGE_SIZE;

    if((unsigned) index >= numPages)
        return -1;

    return index;
}

bool
AddressSpace::NotLoadedPage (unsigned pageIndex)
{
    return pageTable[pageIndex].virtualPage != pageIndex;
}

// GUIDIOS: Es muy chancho esto?
TranslationEntry*
AddressSpace::GetPageTable()
{
    return pageTable;
}

unsigned int
AddressSpace::GetNumPages()
{
    return numPages;
}

void
AddressSpace::LoadPage(unsigned int pageIndex) {
    int physIndex;
    #ifdef DEMAND_LOADING
         physIndex = coreMap -> ReservePage(pageIndex);
         pageTable[pageIndex].physicalPage = physIndex;
    #else
        // This happens only if demand loading is enabled and swap is disabled.
        physIndex = pageTable[pageIndex].physicalPage;
    #endif

    // If the page was never loaded to memory.
    if(pageTable[pageIndex].virtualPage == numPages)
        LoadPageFirst(pageIndex, physIndex);
    // If the page is in the swap file.
    else if(pageTable[pageIndex].virtualPage == numPages + 1)
        LoadPageSwap(pageIndex, physIndex);
}

void
AddressSpace::LoadPageSwap(unsigned int pageIndex, int physIndex)
{
    ASSERT(false);
    return;
}

void
AddressSpace::LoadPageFirst(unsigned int pageIndex, int physIndex)
{
    // GUIDIOS: Hay que ver donde se carga, ahora no hay lugar reservado.
    ASSERT(pageIndex < numPages);

    pageTable[pageIndex].virtualPage = pageIndex;

    unsigned pageStart = pageIndex * PAGE_SIZE;
    unsigned pageEnd = pageStart + PAGE_SIZE;
    // Check for intersection with code segment.
    unsigned codeStart = ourNoffHeader.code.virtualAddr;
    unsigned codeEnd = codeStart + ourNoffHeader.code.size;
    unsigned maxStart = maxx(pageStart, codeStart);
    unsigned minEnd = minn(pageEnd, codeEnd);
    char *mainMemory = machine -> GetMMU() -> mainMemory;

    if(maxStart < minEnd){
        // The intersection is [maxStart, minEnd).
        // Calculate the starting position of the intersection in the file.
        unsigned fileOffset = ourNoffHeader.code.inFileAddr
                              + (maxStart - codeStart);

        // Calculate the starting position of the intersection in the
        // main memory.
        unsigned memoryPosition = physIndex * PAGE_SIZE
                                  + maxStart - pageStart;

        ourExecutable -> ReadAt(&mainMemory[memoryPosition],
                                minEnd - maxStart, fileOffset);
    }

    // Check for intersection with data segment.
    unsigned dataStart = ourNoffHeader.initData.virtualAddr;
    unsigned dataEnd = dataStart + ourNoffHeader.initData.size;
    maxStart = maxx(pageStart, dataStart);
    minEnd = minn(pageEnd, dataEnd);

    if(maxStart < minEnd){
        // The intersection is [maxStart, minEnd).
        // Calculate the starting position of the intersection in the file.
        unsigned fileOffset = ourNoffHeader.initData.inFileAddr
                              + (maxStart - dataStart);

        // Calculate the starting position of the intersection in the
        // main memory.
        unsigned memoryPosition = physIndex * PAGE_SIZE
                                  + maxStart - pageStart;

        ourExecutable -> ReadAt(&mainMemory[memoryPosition],
                                minEnd - maxStart, fileOffset);
    }
}

void
AddressSpace::CopyPageContent(unsigned pageIndex, TranslationEntry* destPage)
{
    ASSERT(pageIndex < numPages);
    *destPage = pageTable[pageIndex];
}
