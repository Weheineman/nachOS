#include "transfer.hh"
#include "lib/utility.hh"
#include "threads/system.hh"


/// Copy a byte array from virtual machine to host.
void ReadBufferFromUser(int userAddress, char *outBuffer,
                        unsigned byteCount)
{
    ASSERT(userAddress != 0);
    ASSERT(outBuffer != nullptr);
    ASSERT(byteCount != 0);
    
    for(unsigned count = 0; count < byteCount; count++, userAddress++){
        int temp;
        ASSERT(machine->ReadMem(userAddress, 1, &temp));
        *outBuffer = (unsigned char) temp;
    }
}

/// Copy a C string from virtual machine to host.
bool ReadStringFromUser(int userAddress, char *outString,
                        unsigned maxByteCount)
{
    ASSERT(userAddress != 0);
    ASSERT(outString != nullptr);
    ASSERT(maxByteCount != 0);

    unsigned count = 0;
    do {
        int temp;
        count++;
        ASSERT(machine->ReadMem(userAddress++, 1, &temp));
        *outString = (unsigned char) temp;
    } while (*outString++ != '\0' && count < maxByteCount);

    return *(outString - 1) == '\0';
}

/// Copy a byte array from host to virtual machine.
void WriteBufferToUser(const char *buffer, int userAddress,
                       unsigned byteCount)
{
    ASSERT(userAddress != 0);
    ASSERT(buffer != nullptr);
    ASSERT(byteCount != 0);
    
    for(unsigned count = 0; count < byteCount; count++, userAddress++, buffer++)
        ASSERT(machine->WriteMem(userAddress, 1, *buffer));
}

/// Copy a C string from host to virtual machine.
void WriteStringToUser(const char *string, int userAddress){
    ASSERT(userAddress != 0);
    ASSERT(string != nullptr);
    
    do {
        ASSERT(machine->WriteMem(userAddress++, 1, *string));
    } while (*string++ != '\0');
}
