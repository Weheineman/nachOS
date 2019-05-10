/// Entry points into the Nachos kernel from user programs.
///
/// There are two kinds of things that can cause control to transfer back to
/// here from user code:
///
/// * System calls: the user code explicitly requests to call a procedure in
///   the Nachos kernel.  Right now, the only function we support is `Halt`.
///
/// * Exceptions: the user code does something that the CPU cannot handle.
///   For instance, accessing memory that does not exist, arithmetic errors,
///   etc.
///
/// Interrupts (which can also cause control to transfer from user code into
/// the Nachos kernel) are handled elsewhere.
///
/// For now, this only handles the `Halt` system call.  Everything else core-
/// dumps.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2019 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "transfer.hh"
#include "syscall.h"
#include "filesys/directory_entry.hh"
#include "threads/system.hh"

void execute_program (void *arg){
	Thread* handlerThread = (Thread *) arg;

	OpenFile *filePtr = fileSystem -> Open(handlerThread -> GetName());

	//GUIDIOS: VER QUÉ HAGO CON EL ASSERT
    ASSERT(filePtr != nullptr);

    AddressSpace *newAddressSpace = new AddressSpace(filePtr);
    handlerThread -> space = newAddressSpace;

	delete filePtr;

    handlerThread -> space -> InitRegisters();  // Set the initial register values.
    handlerThread -> space -> RestoreState();   // Load page table register.

    machine->Run();  // Jump to the user program.
}

static void
IncrementPC()
{
    unsigned pc;

    pc = machine->ReadRegister(PC_REG);
    machine->WriteRegister(PREV_PC_REG, pc);
    pc = machine->ReadRegister(NEXT_PC_REG);
    machine->WriteRegister(PC_REG, pc);
    pc += 4;
    machine->WriteRegister(NEXT_PC_REG, pc);
}

/// Do some default behavior for an unexpected exception.
///
/// * `et` is the kind of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.
static void
DefaultHandler(ExceptionType et)
{
    int exceptionArg = machine->ReadRegister(2);

    fprintf(stderr, "Unexpected user mode exception: %s, arg %d.\n",
            ExceptionTypeToString(et), exceptionArg);
    ASSERT(false);
}

/// Handle a system call exception.
///
/// * `et` is the kind of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.
///
/// The calling convention is the following:
///
/// * system call identifier in `r2`;
/// * 1st argument in `r4`;
/// * 2nd argument in `r5`;
/// * 3rd argument in `r6`;
/// * 4th argument in `r7`;
/// * the result of the system call, if any, must be put back into `r2`.
///
/// And do not forget to increment the program counter before returning. (Or
/// else you will loop making the same system call forever!)
static void
SyscallHandler(ExceptionType _et)
{
    int scid = machine->ReadRegister(2);

    switch (scid) {

        // GUIDIOS: Comentar todos los case
        case SC_HALT:
            DEBUG('a', "Shutdown, initiated by user program.\n");
            interrupt->Halt();
            break;

        // Returns 1 if successful, returns 0 otherwise.
        case SC_CREATE: {
            int filenameAddr = machine->ReadRegister(4);
            if (filenameAddr == 0){
                DEBUG('a', "Error: address to filename string is null.\n");
                break;
            }

            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr, filename, sizeof filename)){
                DEBUG('a', "Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
                break;
            }

            int success = fileSystem -> Create(filename, 0);
            machine -> WriteRegister(2, success);
            DEBUG('a', "Attempted to create file `%s`.\n", filename);
            break;
        }

        // Returns the OpenFileId of the specified file.
        // If there is an error, it returns -1.
        case SC_OPEN: {
            int filenameAddr = machine->ReadRegister(4);
            if (filenameAddr == 0){
                DEBUG('a', "Error: address to filename string is null.\n");
                break;
            }

            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr, filename, sizeof filename)){
                DEBUG('a', "Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
                break;
            }

            OpenFile *filePtr = fileSystem -> Open(filename);
            int possibleFileId = currentThread -> AddFile(filePtr);
            if(possibleFileId == -1)
                DEBUG('a', "Error: fileTable of %s is full.\n",
                      currentThread -> GetName());

            machine -> WriteRegister(2, possibleFileId);
            DEBUG('a', "Request to open file `%s`.\n", filename);
            break;
        }

        case SC_READ: {
            int bufferAddr = machine->ReadRegister(4);
            int readSize = machine->ReadRegister(5);
            OpenFileId fileId = machine->ReadRegister(6);

            if (bufferAddr == 0){
                DEBUG('a', "Error: address to buffer string is null.\n");
                break;
            }

            char *buffer = new char [readSize+1];

            int readBytes = 0;
            if(fileId == CONSOLE_INPUT){
                int ind;
                for(ind = 0; ind < readSize; ind++){
                    buffer[ind] = synchConsole -> GetChar();
                    if(buffer[ind] == '\n')
                        break;
                }
                buffer[ind] = 0;
                readBytes = ind;
            }else{
                if(currentThread -> HasFile(fileId)){
                    OpenFile *filePtr = currentThread -> GetFile(fileId);
                    readBytes = filePtr -> Read(buffer, readSize);
                }else{
                    DEBUG('a', "Error: file with id %d is not open.\n",
                          fileId);
                    break;
                }
            }

            WriteStringToUser(buffer, bufferAddr);
            machine -> WriteRegister(2, readBytes);
            DEBUG('a', "Requested to read %d bytes from file at position %d\n",
            readSize, fileId);

            delete [] buffer;
            break;
        }

        case SC_WRITE: {
            int bufferAddr = machine->ReadRegister(4);
            int writeSize = machine->ReadRegister(5);
            OpenFileId fileId = machine->ReadRegister(6);

            if (bufferAddr == 0){
                DEBUG('a', "Error: address to buffer string is null.\n");
                break;
            }

            if(writeSize == 0){
                DEBUG('a', "Error: writeSize is 0.\n");
                break;
            }

            char *buffer = new char [writeSize + 1];

            if (!ReadStringFromUser(bufferAddr, buffer, writeSize+1)){
                DEBUG('a', "Error: buffer string too long (maximum is %u bytes).\n",
                      writeSize+1);
                break;
            }

            int writtenBytes;
            if(fileId == CONSOLE_OUTPUT){
                int ind;
                for(ind = 0; ind < writeSize and buffer[ind]; ind++)
                    synchConsole -> PutChar(buffer[ind]);
                //synchConsole -> PutChar('\n');
                writtenBytes = ind;
            }else{
                if(currentThread -> HasFile(fileId)){
                    OpenFile *filePtr = currentThread -> GetFile(fileId);
                    writtenBytes = filePtr -> Write(buffer, writeSize);
                }else{
                    DEBUG('a', "Error: file with id %d not open.\n", fileId);
                }
            }

            machine -> WriteRegister(2, writtenBytes);
            DEBUG('a', "Requested to write %d bytes to the file at position %d\n",
                  writeSize, fileId);

            delete [] buffer;
            break;
        }

        case SC_CLOSE: {
            int fileId = machine->ReadRegister(4);

            if(currentThread -> HasFile(fileId))
                currentThread -> RemoveFile(fileId);
            else{
                DEBUG('a', "Error: file %d not open.\n");
            }

            DEBUG('a', "Close requested for id %u.\n", fileId);
            break;
        }

        case SC_EXIT: {
            int exitStatus = machine -> ReadRegister(4);

            DEBUG('a', "Exited with status %d\n", exitStatus);

            currentThread -> Finish(exitStatus);
            break;
        }

        case SC_EXEC:{
            int filenameAddr = machine->ReadRegister(4);
            if (filenameAddr == 0){
                DEBUG('a', "Error: address to filename string is null.\n");
                break;
            }

            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr, filename, sizeof filename)){
                DEBUG('a', "Error: filename string too long (maximum is %u bytes).\n",
                      FILE_NAME_MAX_LEN);
                break;
            }

            // StartProcess(filename);

            // GUIDIOS: Me parece que todo esto son cosas que hay que
            // hacer solo cuando agreguemos args, porque es mas o menos lo
            // que hace StartProcess. Las dejo porque creo que van a servir.
            // All exec threads are joinable
            // GUIDIOS: joinable by default, set by user me parece razonable
            Thread *newThread = new Thread(filename, true);
            SpaceId newSpaceId = newThread -> GetSpaceId();

            newThread -> Fork(execute_program, (void *) newThread);

            machine -> WriteRegister(2, newSpaceId);

            break;
        }

        case SC_JOIN: {
            SpaceId spaceId = machine -> ReadRegister(4);

            if(not threadTable -> HasKey(spaceId)){
                DEBUG('s', "Error: Thread with id %d not found.\n", spaceId);
                break;
            }

            Thread *threadToJoin = threadTable -> Get(spaceId);

            DEBUG('s', "Requested Join with SpaceId %d\n", spaceId);
            int exitStatus = threadToJoin -> Join();

            machine -> WriteRegister(2, exitStatus);
            break;
        }

        default:
            fprintf(stderr, "Unexpected system call: id %d.\n", scid);
            ASSERT(false);

    }

    IncrementPC();
}


/// By default, only system calls have their own handler.  All other
/// exception types are assigned the default handler.
void
SetExceptionHandlers()
{
    machine->SetHandler(NO_EXCEPTION,            &DefaultHandler);
    machine->SetHandler(SYSCALL_EXCEPTION,       &SyscallHandler);
    machine->SetHandler(PAGE_FAULT_EXCEPTION,    &DefaultHandler);
    machine->SetHandler(READ_ONLY_EXCEPTION,     &DefaultHandler);
    machine->SetHandler(BUS_ERROR_EXCEPTION,     &DefaultHandler);
    machine->SetHandler(ADDRESS_ERROR_EXCEPTION, &DefaultHandler);
    machine->SetHandler(OVERFLOW_EXCEPTION,      &DefaultHandler);
    machine->SetHandler(ILLEGAL_INSTR_EXCEPTION, &DefaultHandler);
}
