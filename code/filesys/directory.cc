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
#include "open_file.hh"
#include "lib/utility.hh"
#include "threads/system.hh"
#include "file_system.hh"

/// Initialize a directory; initially, the directory is completely empty.  If
/// the disk is being formatted, an empty directory is all we need, but
/// otherwise, we need to call FetchFrom in order to initialize it from disk.
///
Directory::Directory(int _sector)
{
    directorySize = 0;
    sector = _sector;
    first = last = nullptr;
}

/// De-allocate directory data structure.
Directory::~Directory()
{
    FreeList();
}

/// Read the contents of the directory from disk.
///
/// * `file` is file containing the directory contents.
void
Directory::FetchFrom()
{
    AcquireRead();
    LockedFetchFrom();
    ReleaseRead();
}

/// Write any modifications to the directory back to disk.
///
/// * `file` is a file to contain the new directory contents.
void
Directory::WriteBack()
{
    AcquireRead();
    OpenFile *file = new OpenFile(sector);

    // The file looks like this:
    // [directorySize | DirectoryEntry | ... | DirectoryEntry]
    // where the amount of DirectoryEntries is directorySize

    // First, write the directory size.
    file -> WriteAt((char*) &directorySize, sizeof(unsigned), 0);

    // Then, write the directory entries.
    DirectoryEntry *currentEntry = first;
    for(unsigned writePos = sizeof(unsigned);
        writePos < directorySize * sizeof(DirectoryEntry) + sizeof(unsigned);
        writePos += sizeof(DirectoryEntry)){
        file -> WriteAt((char*) currentEntry, sizeof(DirectoryEntry), writePos);
        currentEntry = currentEntry -> next;
    }

    delete file;
    ReleaseRead();
}

/// Look up file path in directory, and return the disk sector number where
/// the file's header is stored.  Return -1 if the path is not in the
/// directory.
///
/// * `path` is the file path to look up.
int
Directory::Find(const char *pathString)
{
    ASSERT(pathString != nullptr);

    FilePath *path = currentThread -> GetPath();
    path -> Merge(pathString);
    AcquireRead();
    int result = LockedFind(path);

    delete path;
    return result;
}

/// Add a file into the directory.  Return true if successful; return false
/// if the file name is already in the directory, or if the directory is
/// completely full, and has no more space for additional file names.
///
/// * `path` is the path of the file being added.
/// * `newSector` is the disk sector containing the added file's header.
bool
Directory::Add(const char *pathString, int newSector, bool isDirectory)
{
    ASSERT(pathString != nullptr);

    FilePath *path = currentThread -> GetPath();
    path -> Merge(pathString);

    if(path -> IsBottomLevel())
        AcquireWrite();
    else
        AcquireRead();

    bool result = LockedAdd(path, newSector, isDirectory);

    delete path;
    return result;
}

/// Remove a file from the directory.   Return true if successful;
/// return false if the file is not in the directory.
///
/// * `path` is the file path to be removed.
bool
Directory::Remove(const char *pathString)
{
    ASSERT(pathString != nullptr);

    FilePath *path = currentThread -> GetPath();
    path -> Merge(pathString);

    if(path -> IsBottomLevel())
        AcquireWrite();
    else
        AcquireRead();

    bool result = LockedRemove(path);

    delete path;
    return result;
}

/// List all the file names in the current directory.
void
Directory::List()
{
    FilePath *path = currentThread -> GetPath();
    AcquireRead();
    LockedList(path);
    delete path;
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
    // Write the changes back to disk.
    WriteBack();
    directoryLockManager -> ReleaseWrite(sector);
}

// Returns true iff the current directory is empty.
bool
Directory::IsEmpty()
{
    return first == nullptr;
}

/// ASSUMES THE LOCK FOR THE CURRENT DIRECTORY IS TAKEN
/// Find the sector number of the `FileHeader` for file in the given path.
int
Directory::LockedFind(FilePath *path){
    // If the path is root, return the hard coded sector.
    if(path -> IsEmpty()){
        ReleaseRead();
        return DIRECTORY_SECTOR;
    }

    char *currentLevel = nullptr;

    // Initialize to a value that isn't a sector number or an error.
    int sectorNumber = -2;

    while (not path -> IsBottomLevel()){
        if(currentLevel != nullptr)
            delete [] currentLevel;

        currentLevel = path -> SplitBottomLevel();
        DirectoryEntry *entry = LockedFindCurrent(currentLevel);

        // The path has an invalid directory.
        if(entry == nullptr or not entry -> isDirectory){
            sectorNumber = -1;
            break;
        }

        // Replace the data of the directory in RAM with the data of the
        // directory one level below.
        directoryLockManager -> AcquireRead(entry -> sector);
        directoryLockManager -> ReleaseRead(sector);
        sector = entry -> sector;

        // Read the data from disk.
        LockedFetchFrom();
    }

    if(currentLevel != nullptr)
        delete [] currentLevel;

    currentLevel = path -> SplitBottomLevel();

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
bool
Directory::LockedAdd(FilePath *path, int newSector, bool isDirectory){
    // The root folder cannot be created.
    if(path -> IsEmpty()){
        ReleaseWrite();
        return false;
    }

    char *currentLevel = nullptr;

    while (not path -> IsBottomLevel()){
        if(currentLevel != nullptr)
            delete [] currentLevel;

        currentLevel = path -> SplitBottomLevel();
        DirectoryEntry *entry = LockedFindCurrent(currentLevel);

        // The path has an invalid directory.
        if(entry == nullptr or (not entry -> isDirectory)){
            delete [] currentLevel;
            directoryLockManager -> ReleaseRead(sector);
            return false;
        }

        // Replace the data of the directory in RAM with the data of the
        // directory one level below.
        if(path -> IsBottomLevel())
            directoryLockManager -> AcquireWrite(entry -> sector);
        else
            directoryLockManager -> AcquireRead(entry -> sector);

        directoryLockManager -> ReleaseRead(sector);
        sector = entry -> sector;

        // Read the data from disk.
        LockedFetchFrom();
    }

    if(currentLevel != nullptr)
        delete [] currentLevel;

    currentLevel = path -> SplitBottomLevel();

    // The file already exists.
    if(LockedFindCurrent(currentLevel) != nullptr){
        delete [] currentLevel;
        ReleaseWrite();
        return false;
    }

    DirectoryEntry *newEntry =
                    new DirectoryEntry(newSector, isDirectory, currentLevel);

    if(IsEmpty())
        first = last = newEntry;
    else{
        last -> next = newEntry;
        last = last -> next;
    }

    directorySize++;

    // If a new directory was created, it has to be written back as well.
    if(isDirectory){
        directoryLockManager -> AcquireWrite(newSector);
        // Write back the level above the created directory.
        ReleaseWrite();

        sector = newSector;
        directorySize = 0;
        FreeList();
    }

    delete [] currentLevel;
    ReleaseWrite();
    return true;
}


/// ASSUMES THE LOCK FOR THE CURRENT DIRECTORY IS TAKEN
/// Remove a file from the directory.
bool
Directory::LockedRemove(FilePath *path){
    // The root folder cannot be removed.
    if(path -> IsEmpty()){
        ReleaseWrite();
        return false;
    }

    char *currentLevel = nullptr;

    while (not path -> IsBottomLevel()){
        if(currentLevel != nullptr)
            delete [] currentLevel;

        currentLevel = path -> SplitBottomLevel();
        DirectoryEntry *entry = LockedFindCurrent(currentLevel);

        // The path has an invalid directory.
        if(entry == nullptr or not entry -> isDirectory){
            delete [] currentLevel;
            directoryLockManager -> ReleaseRead(sector);
            return false;
        }

        // Replace the data of the directory in RAM with the data of the
        // directory one level below.
        if(path -> IsBottomLevel())
            directoryLockManager -> AcquireWrite(entry -> sector);
        else
            directoryLockManager -> AcquireRead(entry -> sector);

        directoryLockManager -> ReleaseRead(sector);
        sector = entry -> sector;

        // Read the data from disk.
        LockedFetchFrom();
    }

    if(currentLevel != nullptr)
        delete [] currentLevel;

    currentLevel = path -> SplitBottomLevel();

    DirectoryEntry *target = LockedFindCurrent(currentLevel);

    // File not found.
    if(target == nullptr){
        delete [] currentLevel;
        ReleaseWrite();
        return false;
    }

    // Check that the target isn't a non empty directory.
    bool isNonEmptyDir = false;

    if(target -> isDirectory){
        Directory *targetDir = new Directory(target -> sector);
        directoryLockManager -> AcquireRead(target -> sector);
        targetDir -> LockedFetchFrom();
        isNonEmptyDir = targetDir -> IsEmpty();

        delete targetDir;
        directoryLockManager -> ReleaseRead(target -> sector);
    }

    // Can't remove a non empty directory.
    if(isNonEmptyDir){
        delete [] currentLevel;
        ReleaseWrite();
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
		last = current;

    directorySize--;

    delete [] currentLevel;
    ReleaseWrite();
    return true;
}


/// ASSUMES THE LOCK FOR THE CURRENT DIRECTORY IS TAKEN
/// Print the names of all the files in the directory.
void
Directory::LockedList(FilePath *path){
    char *currentLevel = nullptr;

    while(not path -> IsEmpty()){
        if(currentLevel != nullptr)
            delete [] currentLevel;

        currentLevel = path -> SplitBottomLevel();
        DirectoryEntry *entry = LockedFindCurrent(currentLevel);

        // The path has an invalid directory.
        if(entry == nullptr or not entry -> isDirectory){
            printf("Invalid path on LockedList call\n");
            delete [] currentLevel;
            return;
        }

        // Replace the data of the directory in RAM with the data of the
        // directory one level below.
        directoryLockManager -> AcquireRead(entry -> sector);
        directoryLockManager -> ReleaseRead(sector);
        sector = entry -> sector;

        // Read the data from disk.
        LockedFetchFrom();
    }

    printf("Printing directory content:\n");
    for(DirectoryEntry *current = first; current != nullptr;
        current = current -> next)
        printf("%s\n", current -> name);

    if(currentLevel != nullptr)
        delete [] currentLevel;
    directoryLockManager -> ReleaseRead(sector);
}


/// ASSUMES THE LOCK FOR THE CURRENT DIRECTORY IS TAKEN
/// Returns the directory entry corresponding to the given name at the
/// current level.
/// If there isn't one, it returns a nullptr.
DirectoryEntry*
Directory::LockedFindCurrent(const char *name)
{
    // If the name is invalid, it cannot be found.
    if(name == nullptr)
        return nullptr;

    for(DirectoryEntry *currentEntry = first; currentEntry != nullptr;
        currentEntry = currentEntry -> next)
        if(strncmp(currentEntry->name, name, FILE_NAME_MAX_LEN) == 0)
            return currentEntry;

    return nullptr;
}

/// ASSUMES THE LOCK FOR THE CURRENT DIRECTORY IS TAKEN
/// Read the contents of the directory from disk.
///
/// * `file` is file containing the directory contents.
void
Directory::LockedFetchFrom()
{
    // First deallocate the current linked list.
    FreeList();

    // The file looks like this:
    // [directorySize | DirectoryEntry | ... | DirectoryEntry]
    // where the amount of DirectoryEntries is directorySize

    OpenFile *file = new OpenFile(sector);

    // First, get the directory size.
    file -> ReadAt((char*) &directorySize, sizeof(unsigned), 0);

    // Then, read the directory entries.
    for(unsigned readPos = sizeof(unsigned);
        readPos < directorySize * sizeof(DirectoryEntry) + sizeof(unsigned);
        readPos += sizeof(DirectoryEntry)){

        // The constructor is called with dummy values, since they will be
        // overwritten.
        DirectoryEntry *newDirEntry = new DirectoryEntry(0, true, "");
        file -> ReadAt((char*) newDirEntry, sizeof(DirectoryEntry), readPos);

        newDirEntry -> next = nullptr;
        if(IsEmpty())
            first = last = newDirEntry;
        else{
            last -> next = newDirEntry;
            last = last -> next;
        }
    }

    delete file;
}

// Deallocates the memory of the internal linked list.
void
Directory::FreeList()
{
    DirectoryEntry* aux;

    while(first != nullptr){
  		aux = first -> next;
  		delete first;
  		first = aux;
	}
}

// Dummy function our teacher told us not to implement.
void
Directory::Print() const
{
    printf("Hola, persona que corrige! Hablamos con Guillermo y nos dijo que no era necesario implementar print en Directory luego de haber cambiado tan fuertemente su estructura interna. Que tengas un buen dia! ( ͡° ͜ʖ ͡°)\n");
}
