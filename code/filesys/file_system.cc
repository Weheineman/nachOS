/// Routines to manage the overall operation of the file system.  Implements
/// routines to map from textual file names to files.
///
/// Each file in the file system has:
/// * a file header, stored in a sector on disk (the size of the file header
///   data structure is arranged to be precisely the size of 1 disk sector);
/// * a number of data blocks;
/// * an entry in the file system directory.
///
/// The file system consists of several data structures:
/// * A bitmap of free disk sectors (cf. `bitmap.h`).
/// * A directory of file names and file headers.
///
/// Both the bitmap and the directory are represented as normal files.  Their
/// file headers are located in specific sectors (sector 0 and sector 1), so
/// that the file system can find them on bootup.
///
/// The file system assumes that the bitmap and directory files are kept
/// “open” continuously while Nachos is running.
///
/// For those operations (such as `Create`, `Remove`) that modify the
/// directory and/or bitmap, if the operation succeeds, the changes are
/// written immediately back to disk (the two files are kept open during all
/// this time).  If the operation fails, and we have modified part of the
/// directory and/or bitmap, we simply discard the changed version, without
/// writing it back to disk.
///
/// Our implementation at this point has the following restrictions:
///
/// * there is no synchronization for concurrent accesses;
/// * files have a fixed size, set when the file is created;
/// * files cannot be bigger than about 3KB in size;
/// * there is no hierarchical directory structure, and only a limited number
///   of files can be added to the system;
/// * there is no attempt to make the system robust to failures (if Nachos
///   exits in the middle of an operation that modifies the file system, it
///   may corrupt the disk).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2018 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "file_system.hh"
#include "directory.hh"
#include "directory_entry.hh"
#include "file_header.hh"
#include "lib/bitmap.hh"
#include "machine/disk.hh"

/// Initial file sizes for the bitmap.
static const unsigned FREE_MAP_FILE_SIZE = NUM_SECTORS / BITS_IN_BYTE;

/// Initialize the file system.  If `format == true`, the disk has nothing on
/// it, and we need to initialize the disk to contain an empty directory, and
/// a bitmap of free sectors (with almost but not all of the sectors marked
/// as free).
///
/// If `format == false`, we just have to open the files representing the
/// bitmap and the directory.
///
/// * `format` -- should we initialize the disk?
FileSystem::FileSystem(bool format)
{
    DEBUG('f', "Initializing the file system.\n");

    openFileList = new OpenFileList(this);
    freeMapLock = new Lock("freeMap");

    if (format) {
        freeMap   = new Bitmap(NUM_SECTORS);
        Directory  *directory = new Directory(DIRECTORY_SECTOR);
        FileHeader *mapHeader = new FileHeader;
        FileHeader *dirHeader = new FileHeader;

        DEBUG('f', "Formatting the file system.\n");

        // First, allocate space for FileHeaders for the directory and bitmap
        // (make sure no one else grabs these!)
        freeMap->Mark(FREE_MAP_SECTOR);
        freeMap->Mark(DIRECTORY_SECTOR);

        // Second, allocate space for the data blocks containing the contents
        // of the directory and bitmap files.  There better be enough space!

        ASSERT(mapHeader->Allocate(freeMap, FREE_MAP_FILE_SIZE));

        // The directory file looks like this:
        // [directorySize | DirectoryEntry | ... | DirectoryEntry]
        // where the amount of DirectoryEntries is directorySize
        // This means that the inital size is the size of an unsigned variable.
        ASSERT(dirHeader->Allocate(freeMap, sizeof(unsigned)));

        // Flush the bitmap and directory `FileHeader`s back to disk.
        // We need to do this before we can `Open` the file, since open reads
        // the file header off of disk (and currently the disk has garbage on
        // it!).

        DEBUG('f', "Writing headers back to disk.\n");
        mapHeader->WriteBack(FREE_MAP_SECTOR);
        dirHeader->WriteBack(DIRECTORY_SECTOR);

        // OK to open the bitmap and directory files now.
        // The file system operations assume these two files are left open
        // while Nachos is running.

        freeMapFile   = new OpenFile(FREE_MAP_SECTOR);
        directoryFile = new OpenFile(DIRECTORY_SECTOR);

        // Once we have the files “open”, we can write the initial version of
        // each file back to disk.  The directory at this point is completely
        // empty; but the bitmap has been changed to reflect the fact that
        // sectors on the disk have been allocated for the file headers and
        // to hold the file data for the directory and bitmap.

        DEBUG('f', "Writing bitmap back to disk.\n");
        freeMap->WriteBack(freeMapFile);     // flush changes to disk

        DEBUG('f', "Writing directory back to disk.\n");
        directory->WriteBack(directoryFile);

        if (debug.IsEnabled('f')) {
            freeMap->Print();
            // GUIDIOS: Que hacemos con print de FileSystem?
            // directory->Print();
        }

        delete freeMap;
        delete directory;
        delete mapHeader;
        delete dirHeader;
    } else {
        // If we are not formatting the disk, just open the files
        // representing the bitmap and directory; these are left open while
        // Nachos is running.
        freeMapFile   = new OpenFile(FREE_MAP_SECTOR);
        directoryFile = new OpenFile(DIRECTORY_SECTOR);
    }
}

FileSystem::~FileSystem()
{
    delete freeMapFile;
    delete directoryFile;
    delete openFileList;
    delete freeMapLock;
}

/// Create a file in the Nachos file system (similar to UNIX `create`).
/// Since we cannot increase the size of files dynamically, we have to give
/// Create the initial size of the file.
///
/// The steps to create a file are:
/// 1. Make sure the file does not already exist.
/// 2. Allocate a sector for the file header.
/// 3. Allocate space on disk for the data blocks for the file.
/// 4. Add the name to the directory.
/// 5. Store the new file header on disk.
/// 6. Flush the changes to the bitmap and the directory back to disk.
///
/// Return true if everything goes ok, otherwise, return false.
///
/// Create fails if:
/// * file is already in directory;
/// * no free space for file header;
/// * no free entry for file in directory;
/// * no free space for data blocks for the file.
///
/// Note that this implementation assumes there is no concurrent access to
/// the file system!
///
/// * `name` is the name of file to be created.
/// * `initialSize` is the size of file to be created.
bool
FileSystem::Create(const char *name, unsigned initialSize)
{
    ASSERT(name != nullptr);

    Directory  *directory;
    FileHeader *header;
    int         sector;
    bool        success;

    DEBUG('f', "Creating file %s, size %u\n", name, initialSize);

    directory = new Directory(DIRECTORY_SECTOR);
    directory->FetchFrom(directoryFile);

    // GUIDIOS: Chanchadita, cambiar cuando usemos path.
    char *myName = new char [FILE_NAME_MAX_LEN + 1];
    strcpy(myName, name);

    DEBUG('f', "Calling directory -> Find()\n");
    if (directory->Find(myName) != -1)
        success = false;  // File is already in directory.
    else {
        // Redundant because AcquireFreeMap already sets the value of freeMap, but
        // consistent with the use of freeMap outside of FileSystem.
        freeMap = AcquireFreeMap();
        sector = freeMap->Find();  // Find a sector to hold the file header.
        if (sector == -1)
            success = false;    // No free block for file header.
        else if (!directory->Add(myName, sector, false))
            success = false;  // No space in directory.
        else {
            header = new FileHeader;
            if (!header->Allocate(freeMap, initialSize))
                success = false;  // No space on disk for data.
            else {
                success = true;
                // Everthing worked, flush all changes back to disk.
                header->WriteBack(sector);
                directory->WriteBack(directoryFile);
                ReleaseFreeMap(freeMap);
            }
            delete header;
        }
    }
    delete directory;

    return success;
}

/// Open a file for reading and writing.
///
/// To open a file:
/// 1. Find the location of the file's header, using the directory.
/// 2. Bring the header into memory.
///
/// * `name` is the text name of the file to be opened.
OpenFile *
FileSystem::Open(const char *name)
{
    ASSERT(name != nullptr);

    Directory *directory = new Directory(DIRECTORY_SECTOR);
    OpenFile  *openFile = nullptr;
    int        sector;

    DEBUG('f', "Opening file %s\n", name);
    directory->FetchFrom(directoryFile);

    // GUIDIOS: Chanchadita, cambiar cuando usemos path.
    char *myName = new char [FILE_NAME_MAX_LEN + 1];
    strcpy(myName, name);

    DEBUG('f', "Calling dir::find with %s\n", myName);
    sector = directory->Find(myName);
    if (sector >= 0){
        ReaderWriter* newFileRW = openFileList -> AddOpenFile(name);
        if(newFileRW != nullptr)
            openFile = new OpenFile(sector, name, newFileRW);  // `name` was found in directory.
    }

    DEBUG('f', "Open anduvo, %d.\n", openFile == nullptr);
    delete directory;
    return openFile;  // Return null if not found.
}

/// Delete a file from the file system.
///
/// This requires:
/// 1. Remove it from the directory.
/// 2. Delete the space for its header.
/// 3. Delete the space for its data blocks.
/// 4. Write changes to directory, bitmap back to disk.
///
/// Return true if the file was deleted, false if the file was not in the
/// file system.
///
/// * `name` is the text name of the file to be removed.
bool
FileSystem::Remove(const char *name)
{
    ASSERT(name != nullptr);

    bool result = true;
    openFileList -> AcquireListLock();

    // If the file is open, we set its pendingRemove flag.
    // If not, we search for it in the file system and remove it.
    if(not openFileList -> SetUpRemoval(name))
        // Stop other processes from opening the file that will be deleted.
        result = DeleteFromDisk(name);

    openFileList -> ReleaseListLock();
    return result;

}

bool
FileSystem::DeleteFromDisk(const char *name){
    Directory  *directory;
    FileHeader *fileHeader;
    int         sector;

    directory = new Directory(DIRECTORY_SECTOR);
    directory->FetchFrom(directoryFile);

    // GUIDIOS: Chanchadita, cambiar cuando usemos path.
    char *myName = new char [FILE_NAME_MAX_LEN + 1];
    strcpy(myName, name);

    sector = directory->Find(myName);
    if (sector == -1) {
       delete directory;
       return false;  // file not found
    }
    fileHeader = new FileHeader;
    fileHeader->FetchFrom(sector);

    // Redundant because AcquireFreeMap already sets the value of freeMap, but
    // consistent with the use of freeMap outside of FileSystem.
    freeMap = AcquireFreeMap();

    fileHeader->Deallocate(freeMap);  // Remove data blocks.
    freeMap->Clear(sector);           // Remove header block.
    directory->Remove(name);

    ReleaseFreeMap(freeMap);              // Flush to disk.
    directory->WriteBack(directoryFile);  // Flush to disk.
    delete fileHeader;
    delete directory;
    return true;
}

/// Given a thread and a relative path, returns if the global path
/// resulting in merging the thread path with the relative one is
/// valid in the file system. If so, then it also sets the thread path to it.
bool ChangeDirectory(Thread *thread, const char *relativePath){
	/// GUIDIOS: Falta el lock stuff y qué sé yo.
    return true;
	// FilePath *threadPath = thread -> GetPath();
	// threadPath -> Merge(relativePath);
    //
	// char *newPath = threadPath -> ToString();
	// bool dirFound = (Find(newPath) != -1);
    //
	// if(dirFound)
	// 	thread -> SetPath(threadPath);
	// else
	// 	delete threadPath;
    //
	// delete [] newPath;
	// return dirFound;
}

/// List all the files in the file system directory.
void
FileSystem::List()
{
    Directory *directory = new Directory(DIRECTORY_SECTOR);

    directory->FetchFrom(directoryFile);

    DEBUG('f', "Directory fetched from file\n");

    // GUIDIOS: FIX FIX FIX FIX FIX
    // GUIDIOS: Chanchadita, cambiar cuando usemos path.
    char *myName = new char [FILE_NAME_MAX_LEN + 1];
    strcpy(myName, "/");

    directory->List(myName);
    delete directory;
}

// static bool
// AddToShadowBitmap(unsigned sector, Bitmap *map)
// {
//     ASSERT(map != nullptr);
//
//     if (map->Test(sector)) {
//         DEBUG('f', "Sector %u was already marked.\n", sector);
//         return false;
//     }
//     map->Mark(sector);
//     DEBUG('f', "Marked sector %u.\n", sector);
//     return true;
// }

// static bool
// CheckForError(bool value, const char *message)
// {
//     if (!value)
//         DEBUG('f', message);
//     return !value;
// }

// static bool
// CheckSector(unsigned sector, Bitmap *shadowMap)
// {
//     bool error = false;
//
//     error |= CheckForError(sector < NUM_SECTORS, "Sector number too big.\n");
//     error |= CheckForError(AddToShadowBitmap(sector, shadowMap),
//                            "Sector number already used.\n");
//     return error;
// }
//
// static bool
// CheckFileHeader(const RawFileHeader *rh, unsigned num, Bitmap *shadowMap)
// {
//     ASSERT(rh != nullptr);
//
//     bool error = false;
//
//     DEBUG('f', "Checking file header %u.  File size: %u bytes, number of sectors: %u.\n",
//           num, rh->numBytes, rh->numSectors);
//     error |= CheckForError(rh->numSectors >= DivRoundUp(rh->numBytes,
//                                                         SECTOR_SIZE),
//                            "Sector count not compatible with file size.\n");
//     error |= CheckForError(rh->numSectors < NUM_DIRECT,
// 		           "Too many blocks.\n");
//     for (unsigned i = 0; i < rh->numSectors; i++) {
//         unsigned s = rh->dataSectors[i];
//         error |= CheckSector(s, shadowMap);
//     }
//     return error;
// }
//
// static bool
// CheckBitmaps(const Bitmap *freeMap, const Bitmap *shadowMap)
// {
//     bool error = false;
//     for (unsigned i = 0; i < NUM_SECTORS; i++) {
//         DEBUG('f', "Checking sector %u. Original: %u, shadow: %u.\n",
//               i, freeMap->Test(i), shadowMap->Test(i));
//         error |= CheckForError(freeMap->Test(i) == shadowMap->Test(i),
//                                "Inconsistent bitmap.");
//     }
//     return error;
// }

// static bool
// CheckDirectory(const RawDirectory *rd, Bitmap *shadowMap)
// {
//     ASSERT(rd != nullptr);
//     ASSERT(shadowMap != nullptr);
//
//     bool error = false;
//     unsigned nameCount = 0;
//     const char *knownNames[NUM_DIR_ENTRIES];
//
//     for (unsigned i = 0; i < NUM_DIR_ENTRIES; i++) {
//         DEBUG('f', "Checking direntry: %u.\n", i);
//         const DirectoryEntry *e = &rd->table[i];
//
//         if (e->inUse) {
//             if (strlen(e->name) > FILE_NAME_MAX_LEN) {
//                 DEBUG('f', "Filename too long.\n");
//                 error = true;
//             }
//
//             // Check for repeated filenames.
//             DEBUG('f', "Checking for repeated names.  Name count: %u.\n",
//                   nameCount);
//             bool repeated = false;
//             for (unsigned j = 0; j < nameCount; j++) {
//                 DEBUG('f', "Comparing \"%s\" and \"%s\".\n",
//                       knownNames[j], e->name);
//                 if (strcmp(knownNames[j], e->name) == 0) {
//                     DEBUG('f', "Repeated filename.\n");
//                     repeated = true;
//                     error = true;
//                 }
//             }
//             if (!repeated) {
//                 knownNames[nameCount] = e->name;
//                 DEBUG('f', "Added \"%s\" at %u.\n", e->name, nameCount);
//                 nameCount++;
//             }
//
//             // Check sector.
//             error |= CheckSector(e->sector, shadowMap);
//
//             // Check file header.
//             FileHeader *h = new FileHeader;
//             const RawFileHeader *rh = h->GetRaw();
//             h->FetchFrom(e->sector);
//             error |= CheckFileHeader(rh, e->sector, shadowMap);
//             delete h;
//         }
//     }
//     return error;
// }
//
// bool
// FileSystem::Check()
// {
//     DEBUG('f', "Performing filesystem check\n");
//     bool error = false;
//
//     Bitmap *shadowMap = new Bitmap(NUM_SECTORS);
//     shadowMap->Mark(FREE_MAP_SECTOR);
//     shadowMap->Mark(DIRECTORY_SECTOR);
//
//     DEBUG('f', "Checking bitmap's file header.\n");
//
//     FileHeader *bitH = new FileHeader;
//     const RawFileHeader *bitRH = bitH->GetRaw();
//     bitH->FetchFrom(FREE_MAP_SECTOR);
//     DEBUG('f', "  File size: %u bytes, expected %u bytes.\n"
//                "  Number of sectors: %u, expected %u.\n",
//           bitRH->numBytes, FREE_MAP_FILE_SIZE,
//           bitRH->numSectors, FREE_MAP_FILE_SIZE / SECTOR_SIZE);
//     error |= CheckForError(bitRH->numBytes == FREE_MAP_FILE_SIZE,
//                            "Bad bitmap header: wrong file size.\n");
//     error |= CheckForError(bitRH->numSectors == FREE_MAP_FILE_SIZE / SECTOR_SIZE,
//                            "Bad bitmap header: wrong number of sectors.\n");
//     error |= CheckFileHeader(bitRH, FREE_MAP_SECTOR, shadowMap);
//     delete bitH;
//
//     DEBUG('f', "Checking directory.\n");
//
//     FileHeader *dirH = new FileHeader;
//     const RawFileHeader *dirRH = dirH->GetRaw();
//     dirH->FetchFrom(DIRECTORY_SECTOR);
//     error |= CheckFileHeader(dirRH, DIRECTORY_SECTOR, shadowMap);
//     delete dirH;
//
//     Bitmap *freeMap = AcquireFreeMap(false);
//     Directory *dir = new Directory(NUM_DIR_ENTRIES);
//     const RawDirectory *rdir = dir->GetRaw();
//     dir->FetchFrom(directoryFile);
//     error |= CheckDirectory(rdir, shadowMap);
//     delete dir;
//
//     // The two bitmaps should match.
//     DEBUG('f', "Checking bitmap consistency.\n");
//     error |= CheckBitmaps(freeMap, shadowMap);
//     delete shadowMap;
//     ReleaseFreeMap(freeMap, false);
//
//     DEBUG('f', error ? "Filesystem check succeeded.\n"
//                      : "Filesystem check failed.\n");
//
//     return !error;
// }

/// Print everything about the file system:
/// * the contents of the bitmap;
/// * the contents of the directory;
/// * for each file in the directory:
///   * the contents of the file header;
///   * the data in the file.
void
FileSystem::Print()
{

    FileHeader *bitHeader = new FileHeader;
    FileHeader *dirHeader = new FileHeader;
    freeMap   = AcquireFreeMap();
    Directory  *directory = new Directory(DIRECTORY_SECTOR);

    printf("--------------------------------\n"
           "Bit map file header:\n\n");
    bitHeader->FetchFrom(FREE_MAP_SECTOR);
    bitHeader->Print();

    printf("--------------------------------\n"
           "Directory file header:\n\n");
    dirHeader->FetchFrom(DIRECTORY_SECTOR);
    dirHeader->Print();

    printf("--------------------------------\n");
    freeMap->Print();

    printf("--------------------------------\n");
    directory->FetchFrom(directoryFile);
    printf("--------------------------------\n");

    delete bitHeader;
    delete dirHeader;
    ReleaseFreeMap(freeMap);
    delete directory;
}

/// Returns the bitmap of free sectors on the disk granting reading and
/// writing exclusivity.
Bitmap *
FileSystem::AcquireFreeMap()
{
    freeMapLock -> Acquire();

    freeMap = new Bitmap(NUM_SECTORS);
    freeMap -> FetchFrom(freeMapFile);

    return freeMap;
}

/// Returns the current value of the freeMap pointer. No exclusive access
/// is guaranteed.
Bitmap*
FileSystem::GetCurrentFreeMap(){
    return freeMap;
}

/// Marks the end of the freeMap usage. The lock is released, the
/// memory is freed and the changes are saved to disk.
void
FileSystem::ReleaseFreeMap(Bitmap *freeMap_)
{
    ASSERT(freeMap_ == freeMap);

    freeMap -> WriteBack(freeMapFile);
    delete freeMap;
    freeMap = nullptr;

    freeMapLock -> Release();
}

void
FileSystem::CloseFile(const char *name)
{
    openFileList -> CloseOpenFile(name);
}
