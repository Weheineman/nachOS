#include "threads/thread.hh"
#include "threads/system.hh"
#include "address_space.hh"


void SequentialSetup(void *filename_){
	char *filename = (char *) filename_;
	ASSERT(filename != nullptr);

    OpenFile *executable = fileSystem->Open(filename);
    if (executable == nullptr) {
        printf("Unable to open file %s\n", filename);
        currentThread -> Finish();
    }
    // Set the new Address Space for the thread.
    currentThread -> InitAddressSpace(executable);

    currentThread -> GetAddressSpace() -> InitRegisters();  // Set the initial register values.
    currentThread -> GetAddressSpace() -> RestoreState();   // Load page table register.

    machine->Run();  // Jump to the user progam.
}

void TestSequentialProcesses(int processAmount){
	char name[64];
	char processName[] = "../userland/matmult";
	for(int processNum = 1; processNum <= processAmount; processNum++){
		snprintf(name, 64, "%s%d", "Number ", processNum);
		Thread *newThread = new Thread(name, true, 0);
		DEBUG('v', "About to fork Process %s\n", name);
		newThread->Fork(SequentialSetup, processName);
		DEBUG('v', "About to join Process %s\n", name);
		int status = newThread->Join();
		DEBUG('v', "Process %s returned: %d\n", name, status);
	}
	DEBUG('v', "Exiting Sequential Processes test.\n");
}

void TestConcurrentProcesses(int processAmount){
	char name[64];
	char processName[] = "../userland/matmult";
	Thread* sons[processAmount];
	for(int processNum = 1; processNum <= processAmount; processNum++){
		snprintf(name, 64, "%s%d", "Number ", processNum);
		sons[processNum - 1] = new Thread(name, true, 0);
		DEBUG('v', "About to fork Process %s\n", name);
		sons[processNum - 1]->Fork(SequentialSetup, processName);
	}
	for(int processNum = 1; processNum <= processAmount; processNum++){
		DEBUG('v', "About to join Process %d\n", processNum);
		int status = sons[processNum - 1]->Join();
		DEBUG('v', "Process %d returned: %d\n", processNum, status);
	}

	DEBUG('v', "Exiting Concurrent Processes test.\n");
}
