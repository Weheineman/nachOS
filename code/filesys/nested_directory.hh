/// Data structures to manage a UNIX-like directory of file paths.
///
/// A directory is a linked list of DirectoryEntry nodes.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_FILESYS_DIRECTORY__HH
#define NACHOS_FILESYS_DIRECTORY__HH


#include "raw_directory.hh"
#include "open_file.hh"


/// The following class defines a UNIX-like “directory”.  Each entry in the
/// directory describes a file, and where to find it on disk.
///
/// The directory data structure can be stored in memory, or on disk.  When
/// it is on disk, it is stored as a regular Nachos file.
///
/// The constructor initializes a directory structure in memory; the
/// `FetchFrom`/`WriteBack` operations shuffle the directory information
/// from/to disk.
class Directory {
public:
    /// Initialize an empty directory.
    Directory();

    /// De-allocate the directory.
    ~Directory();

    /// Initialize directory contents from disk.
    void FetchFrom(OpenFile *file);

    /// Write modifications to directory contents back to disk.
    void WriteBack(OpenFile *file);

    /// Find the sector number of the `FileHeader` for file in the given path.
    int Find(const char *path);

    /// Add a file into the directory at the given path.
    bool Add(const char *path, int newSector);

    /// Remove a file from the directory.
    bool Remove(const char *path);

    /// Print the names of all the files in the directory.
    void List() const;

    /// Verbose print of the contents of the directory -- all the file paths
    /// and their contents.
    void Print() const;

    // Interface for the ReaderWriter lock
    void AcquireRead();

    void AcquireWrite();

    void ReleaseRead();

    void ReleaseWrite();

    // Returns true iff the current directory is empty.
    bool IsEmpty();

private:
    DirectoryEntry *first, *last;

    // Size of the linked list.
    unsigned directorySize;

    // Sector of the file header of the directory. Used as an argument for the
    // DirectoryLockManager methods.
    int sector;

    /// ASSUMES THE LOCK FOR THE CURRENT DIRECTORY IS TAKEN
    /// Find the sector number of the `FileHeader` for file in the given path.
    int LockedFind(const char *path);

    /// ASSUMES THE LOCK FOR THE CURRENT DIRECTORY IS TAKEN
    /// Add a file into the directory at the given path.
    bool LockedAdd(const char *path, int newSector, bool isDirectory);

    /// ASSUMES THE LOCK FOR THE CURRENT DIRECTORY IS TAKEN
    /// Remove a file from the directory.
    bool LockedRemove(const char *path);

    /// ASSUMES THE LOCK FOR THE CURRENT DIRECTORY IS TAKEN
    /// Print the names of all the files in the directory.
    void LockedList() const;

    /// ASSUMES THE LOCK FOR THE CURRENT DIRECTORY IS TAKEN
    /// Returns the directory entry corresponding to the given name at the
    /// current level.
    /// If there isn't one, it returns a nullptr.
    DirectoryEntry* LockedFindCurrent(const char *name);

    // Removes '/' from the first position in the string.
    // Returns true if successful.
    // Returns false if there was no '/' at the first position.
    bool RemoveFirstSlash(char *path);

    // Returns true iff the path does not have more than one level:
    //     "knuth" returns true
    //     "knuth/books" returns false
    bool IsBottomLevel(char *path);

    // Returns the top level file name of the path and removes it from the path:
    //     "knuth/books" returns "knuth" and changes path to "books"
    char* SplitCurrentLevel(char *path);
};

#endif
