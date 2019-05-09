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
#include "bin/noff.h"
#include "machine/endianness.hh"
#include "threads/system.hh"
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

    ASSERT(numPages <= pageMap -> CountClear());
      // Check we are not trying to run anything too big.

    DEBUG('a', "Initializing address space, num pages %u, size %u\n",
          numPages, size);

    // First, set up the translation.
    char *mainMemory = machine->GetMMU()->mainMemory;

    pageTable = new TranslationEntry[numPages];
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

}

/// Deallocate an address space.
///
/// Nothing for now!
AddressSpace::~AddressSpace()
{
	for(unsigned i = 0; i < numPages; i++)
		pageMap -> Clear(pageTable[i].physicalPage);

    delete [] pageTable;
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
///
/// For now, nothing!
void
AddressSpace::SaveState()
{}

/// On a context switch, restore the machine state so that this address space
/// can run.
///
/// For now, tell the machine where to find the page table.
void
AddressSpace::RestoreState()
{
    machine->GetMMU()->pageTable     = pageTable;
    machine->GetMMU()->pageTableSize = numPages;
}
