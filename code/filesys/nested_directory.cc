/// Routines to manage a directory of file names.
///
/// The directory is a table of fixed length entries; each entry represents a
/// single file, and contains the file name, and the location of the file
/// header on disk.  The fixed size of each directory entry means that we
/// have the restriction of a fixed maximum size for file names.
///
/// The constructor initializes an empty directory of a certain size; we use
/// ReadFrom/WriteBack to fetch the contents of the directory from disk, and
/// to write back any modifications back to disk.
///
/// Also, this implementation has the restriction that the size of the
/// directory cannot expand.  In other words, once all the entries in the
/// directory are used, no more files can be created.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2018 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "directory.hh"
#include "directory_entry.hh"
#include "file_header.hh"
#include "openfile.hh"
#include "lib/utility.hh"
#include "threads/system.hh"


/// Initialize a directory; initially, the directory is completely empty.  If
/// the disk is being formatted, an empty directory is all we need, but
/// otherwise, we need to call FetchFrom in order to initialize it from disk.
///
Directory::Directory(int _sector)
{
    sector = _sector;
    first = last = nullptr;
}

/// De-allocate directory data structure.
Directory::~Directory()
{
    DirectoryEntry* aux;

    // GUIDIOS: Hay que ver como cambiar esto para directorios
    // recursivos, queda asi de placeholder.
    // Ver bien cuando se llama al destructor.
    while(first != nullptr){
  		aux = first -> next;
  		delete first;
  		first = aux;
	}
}

/// Read the contents of the directory from disk.
///
/// * `file` is file containing the directory contents.
void
Directory::FetchFrom(OpenFile *file)
{
    ASSERT(file != nullptr);

    AcquireWrite();

    // The file looks like this:
    // [directorySize | DirectoryEntry | ... | DirectoryEntry]
    // where the amount of DirectoryEntries is directorySize

    // First, get the directory size.
    file -> ReadAt((char*) &directorySize, sizeof unsigned, 0);

    // Then, read the directory entries.
    for(unsigned readPos = sizeof unsigned;
        readPos < directorySize * (sizeof DirectoryEntry) + sizeof unsigned;
        readPos += sizeof DirectoryEntry){
        DirectoryEntry *newDirEntry = new DirectoryEntry;
        file -> ReadAt((char*) newDirEntry, sizeof DirectoryEntry, readPos);

        newDirEntry -> next = nullptr;
        if(IsEmpty())
            first = last = newDirEntry;
        else{
            last -> next = newDirEntry;
            last = last -> next;
        }
    }

    ReleaseWrite();
}

/// Write any modifications to the directory back to disk.
///
/// * `file` is a file to contain the new directory contents.
void
Directory::WriteBack(OpenFile *file)
{
    // GUIDIOS: Solo escribe el nivel actual. Queremos una version recursiva?

    ASSERT(file != nullptr);
    // GUIDIOS: Si fuera necesario extender el archivo aca, tener cuidado
    // con tomar el freeMap dentro de WriteAt
    // SI, HAY QUE CAMBIAR WRITEAT

    AcquireRead();

    // The file looks like this:
    // [directorySize | DirectoryEntry | ... | DirectoryEntry]
    // where the amount of DirectoryEntries is directorySize

    // First, write the directory size.
    file -> WriteAt((char*) &directorySize, sizeof unsigned, 0);

    // Then, write the directory entries.
    DirectoryEntry *currentEntry = first;
    for(unsigned writePos = sizeof unsigned;
        writePos < directorySize * (sizeof DirectoryEntry) + sizeof unsigned;
        writePos += sizeof DirectoryEntry){
        file -> WriteAt((char*) currentEntry, sizeof DirectoryEntry, writePos);
        currentEntry = currentEntry -> next;
    }

    ReleaseRead();
}

/// Look up file path in directory, and return the disk sector number where
/// the file's header is stored.  Return -1 if the path is not in the
/// directory.
///
/// * `path` is the file path to look up.
int
Directory::Find(const char *path)
{
    ASSERT(path != nullptr);

    if(not RemoveFirstSlash(path))
        return -1;

    AcquireRead();

    return LockedFind(path);
}

/// Add a file into the directory.  Return true if successful; return false
/// if the file name is already in the directory, or if the directory is
/// completely full, and has no more space for additional file names.
///
/// * `path` is the path of the file being added.
/// * `newSector` is the disk sector containing the added file's header.
bool
Directory::Add(const char *path, int newSector)
{
    ASSERT(path != nullptr);

    if(not RemoveFirstSlash(path))
        return false;

    AcquireWrite();

    return LockedAdd(path, newSector);
}

/// Remove a file from the directory.   Return true if successful;
/// return false if the file is not in the directory.
///
/// * `path` is the file path to be removed.
bool
Directory::Remove(const char *path)
{
    ASSERT(path != nullptr);

    if(not RemoveFirstSlash(path))
        return false;

    AcquireWrite();

    return LockedRemove(path);
}

/// List all the file names in the directory.
void
Directory::List() const
{
    if(not RemoveFirstSlash(path))
        return;

    AcquireRead();

    LockedList();
}

/// List all the file names in the directory, their `FileHeader` locations,
/// and the contents of each file.  For debugging.
void
Directory::Print() const
{
    FileHeader *hdr = new FileHeader;

    printf("Directory contents:\n");
    for (unsigned i = 0; i < raw.tableSize; i++)
        if (raw.table[i].inUse) {
            printf("\nDirectory entry.\n"
                   "    Name: %s\n"
                   "    Sector: %u\n",
                   raw.table[i].name, raw.table[i].sector);
            hdr->FetchFrom(raw.table[i].sector);
            hdr->Print();
        }
    printf("\n");
    delete hdr;
}

// Interface for the ReaderWriter lock
void
Directory::AcquireRead()
{
    directoryLockManager -> AcquireRead(sector);
}

void
Directory::AcquireWrite()
{
    directoryLockManager -> AcquireWrite(sector);
}

void
Directory::ReleaseRead()
{
    directoryLockManager -> ReleaseRead(sector);
}

void
Directory::ReleaseWrite()
{
    directoryLockManager -> ReleaseWrite(sector);
}

// Returns true iff the current directory is empty.
bool
Directory::IsEmpty()
{
    return directorySize == 0;
}

/// ASSUMES THE LOCK FOR THE CURRENT DIRECTORY IS TAKEN
/// Find the sector number of the `FileHeader` for file in the given path.
int LockedFind(const char *path){
    char *currentLevel = new char [FILE_NAME_MAX_LEN + 1];
    int sectorNumber = -2;

    while (not IsBottomLevel(path)){
        strncpy(currentLevel, SplitCurrentLevel(path), FILE_NAME_MAX_LEN+1);
        DirectoryEntry *entry = LockedFindCurrent(currentLevel);

        // The path has an invalid directory.
        if(not entry -> isDirectory){
            sectorNumber = -1;
            break;
        }

        // Replace the data of the directory in RAM with the data of the
        // directory one level below.
        directoryLockManager -> AcquireRead(entry -> sector);
        directoryLockManager -> ReleaseRead(sector);
        sector = entry -> sector;

        // Read the data from disk.
        OpenFile *dirFile = new OpenFile(sector);
        FetchFrom(dirFile);
        delete dirFile;
    }

    DirectoryEntry *entry = LockedFindCurrent(currentLevel);

    // The bottom level file does not exist.
    if(entry == nullptr)
        sectorNumber = -1;
    else
        // If there were no errors, get the sector number.
        if(sectorNumber != -1)
            sectorNumber = entry -> sector;

    delete [] currentLevel;
    directoryLockManager -> ReleaseRead(sector);
    return sectorNumber;
}


/// ASSUMES THE LOCK FOR THE CURRENT DIRECTORY IS TAKEN
/// Add a file into the directory at the given path.
bool LockedAdd(const char *path, int newSector, bool isDirectory){
    char *currentLevel = new char [FILE_NAME_MAX_LEN + 1];
    bool bottomLevel = IsBottomLevel(path);

    while (not bottomLevel){
        strncpy(currentLevel, SplitCurrentLevel(path), FILE_NAME_MAX_LEN+1);
        bottomLevel = IsBottomLevel(path);
        DirectoryEntry *entry = LockedFindCurrent(currentLevel);

        // The path has an invalid directory.
        if(not entry -> isDirectory){
            delete [] currentLevel;
            directoryLockManager -> ReleaseRead(sector);
            return false;
        }

        // Replace the data of the directory in RAM with the data of the
        // directory one level below.
        if(bottomLevel)
            directoryLockManager -> AcquireWrite(entry -> sector);
        else
            directoryLockManager -> AcquireRead(entry -> sector);

        directoryLockManager -> ReleaseRead(sector);
        sector = entry -> sector;

        // Read the data from disk.
        OpenFile *dirFile = new OpenFile(sector);
        FetchFrom(dirFile);
        delete dirFile;
    }

    // The file already exists.
    if(LockedFindCurrent(path) != nullptr){
        delete [] currentLevel;
        directoryLockManager -> ReleaseWrite(sector);
        return false;
    }

    DirectoryEntry *newEntry = new DirectoryEntry(newSector, isDirectory);
    if(IsEmpty())
        first = last = newEntry;
    else{
        last -> next = newEntry;
        last = last -> next;
    }

    directorySize++;

    delete [] currentLevel;
    directoryLockManager -> ReleaseWrite(sector);
    return true;
}


/// ASSUMES THE LOCK FOR THE CURRENT DIRECTORY IS TAKEN
/// Remove a file from the directory.
bool LockedRemove(const char *path){
    char *currentLevel = new char [FILE_NAME_MAX_LEN + 1];
    bool bottomLevel = IsBottomLevel(path);

    while (not bottomLevel){
        strncpy(currentLevel, SplitCurrentLevel(path), FILE_NAME_MAX_LEN+1);
        bottomLevel = IsBottomLevel(path);
        DirectoryEntry *entry = LockedFindCurrent(currentLevel);

        // The path has an invalid directory.
        if(not entry -> isDirectory){
            delete [] currentLevel;
            directoryLockManager -> ReleaseRead(sector);
            return false;
        }

        // Replace the data of the directory in RAM with the data of the
        // directory one level below.
        if(bottomLevel)
            directoryLockManager -> AcquireWrite(entry -> sector);
        else
            directoryLockManager -> AcquireRead(entry -> sector);

        directoryLockManager -> ReleaseRead(sector);
        sector = entry -> sector;

        // Read the data from disk.
        OpenFile *dirFile = new OpenFile(sector);
        FetchFrom(dirFile);
        delete dirFile;
    }

    DirectoryEntry *target = LockedFindCurrent(path);

    // File not found.
    if(target == nullptr){
        delete [] currentLevel;
        directoryLockManager -> ReleaseWrite(sector);
        return false;
    }

    // Check that the target isn't a non empty directory.
    bool isNonEmptyDir = false;

    if(target -> isDirectory){
        Directory targetDir = new Directory();
        directoryLockManager -> AcquireRead(target -> sector);
        OpenFile *dirFile = new OpenFile(target -> sector);
        targetDir -> FetchFrom(dirFile);
        isNonEmptyDir = targetDir -> IsEmpty();

        delete dirFile;
        delete targetDir;
        directoryLockManager -> ReleaseRead(target -> sector);
    }

    // Can't remove a non empty directory.
    if(isNonEmptyDir){
        delete [] currentLevel;
        directoryLockManager -> ReleaseWrite(sector);
        return false;
    }

    DirectoryEntry *current = nullptr;

    //If the first item is to be deleted, advance the first pointer.
	if(first == target)
		first = first -> next;
	else{
		for(current = first; current -> next != target;
            current = current -> next);

		current -> next = current -> next -> next;
	}

	//If the last item is to be deleted, bring the last pointer one item back.
	if(last == target)
		last = aux;

    directorySize--;

    delete [] currentLevel;
    directoryLockManager -> ReleaseWrite(sector);
    return true;
}

/// ASSUMES THE LOCK FOR THE CURRENT DIRECTORY IS TAKEN
/// Returns the directory entry corresponding to the given name at the
/// current level.
/// If there isn't one, it returns a nullptr.
DirectoryEntry*
Directory::LockedFindCurrent(const char *name)
{
    for(DirectoryEntry currentEntry = first; currentEntry != nullptr;
        currentEntry = currentEntry -> next)
        if(strncmp(currentEntry->name, name, FILE_NAME_MAX_LEN) == 0)
            return currentEntry;

    return nullptr;
}

// GUIDIOS: Y si nos kackean?
// Removes '/' from the first position in the string.
// Returns true if successful.
// Returns false if there was no '/' at the first position.
bool
Directory::RemoveFirstSlash(char *path)
{
    if(path[0] != '/')
        return false;

    // We have well behaved users who will not hack us :)
    unsigned length = strlen(path);

    for(unsigned ind = 1; ind < length; ind++)
        path[ind-1] = path[ind];

    return true;
}

// Returns true iff the path does not have more than one level:
//     "knuth" returns true
//     "knuth/books" returns false
bool
Directory::IsBottomLevel(char *path)
{
    // "filename/" has length at most FILE_NAME_MAX_LEN + 1
    for(unsigned ind = 0; ind < FILE_NAME_MAX_LEN + 1; ind++){
        if(path[ind] == '\0')
            break;

        if(path[ind] == '/')
            return false;
    }

    return true;
}

// Returns the top level file name of the path and removes it from the path:
//     "knuth/books" returns "knuth" and changes path to "books"
char*
Directory::SplitCurrentLevel(char *path)
{
    char* currentLevel = new char[FILE_NAME_MAX_LEN];

    // The current level is the prefix up to the first '/' or
    // string terminator.
    unsigned ind = 0;
    for(; ind < FILE_NAME_MAX_LEN; ind++){
        if(path[ind] == '\0' or path[ind] == '/')
            break;

        currentLevel[ind] = path[ind];
    }
    currentLevel[ind] = '\0';

    // We have well behaved users who will not hack us :)
    unsigned length = strlen(path);

    // Remove the trailing '/'
    if(path[ind] == '/')    ind++;

    // Move the contents of path to eliminate the prefix that was split.
    for(unsigned pos = ind; pos < length; pos++)
        path[pos - ind] = path[pos];
}
