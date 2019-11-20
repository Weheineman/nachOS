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


#include "directory_entry.hh"
#include "open_file.hh"
#include "file_path.hh"

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
    Directory(int _sector);

    /// De-allocate the directory.
    ~Directory();

    /// Initialize directory contents from disk.
    void FetchFrom(OpenFile *file);

    /// Write modifications to directory contents back to disk.
    void WriteBack(OpenFile *file);

    /// Find the sector number of the `FileHeader` for file in the given path.
    int Find(const char *path);

    /// Add a file or directory into the directory at the given path.
    /// If isDirectory is true, a directory is added.
    /// Otherwise, a non directory file is added.
    bool Add(const char *path, int newSector, bool isDirectory);

    /// Remove a file from the directory.
    bool Remove(const char *path);

    /// Print the names of all the files in the directory.
    void List(const char *path);

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
    int LockedFind(FilePath *path);

    /// ASSUMES THE LOCK FOR THE CURRENT DIRECTORY IS TAKEN
    /// Add a file into the directory at the given path.
    bool LockedAdd(FilePath *path, int newSector, bool isDirectory);

    /// ASSUMES THE LOCK FOR THE CURRENT DIRECTORY IS TAKEN
    /// Remove a file from the directory.
    bool LockedRemove(FilePath *path);

    /// ASSUMES THE LOCK FOR THE CURRENT DIRECTORY IS TAKEN
    /// Print the names of all the files in the directory.
    void LockedList(FilePath *path);

    /// ASSUMES THE LOCK FOR THE CURRENT DIRECTORY IS TAKEN
    /// Returns the directory entry corresponding to the given name at the
    /// current level.
    /// If there isn't one, it returns a nullptr.
    DirectoryEntry* LockedFindCurrent(const char *name);
};

#endif
