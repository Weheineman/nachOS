/// Data structures to manage a UNIX-like directory of file paths.
///
/// A directory is a table of pairs: *<file path, sector #>*, giving the path
/// of each file in the directory, and where to find its file header (the
/// data structure describing where to find the file's data blocks) on disk.
///
/// We assume mutual exclusion is provided by the caller.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_FILESYS_DIRECTORY__HH
#define NACHOS_FILESYS_DIRECTORY__HH


#include "raw_directory.hh"
#include "reader_writer.hh"
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

    /// Get the raw directory structure.
    ///
    /// NOTE: this should only be used by routines that operating on the file
    /// system at a low level.
    const RawDirectory *GetRaw() const;

private:
    /// The directory directly above the current level.
    /// GUIDIOS: Esta bien asi? Podriamos como alernativa
    /// pasar como argument a Find, Add y Remove el puntero
    /// del directORIo a deletear.
    Directory *upper;

    DirectoryEntry *first, *last;

    ReaderWriter *lock;

    /// ASSUMES THE LOCK FOR THE CURRENT DIRECTORY IS TAKEN
    /// Find the sector number of the `FileHeader` for file in the given path.
    int LockedFind(const char *path);

    /// ASSUMES THE LOCK FOR THE CURRENT DIRECTORY IS TAKEN
    /// Add a file into the directory at the given path.
    /// GUIDIOS: OJO con la siguiente situacion: / -> A -> B
    /// Se puede borrar A aunque un thread este en B.
    /// En ese caso, cuando B quiera hacer una operacion, no tiene que poder.
    bool LockedAdd(const char *path, int newSector);

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

    /// Acquires writing permission of the current directory and all its
    /// descendants.
    bool SetUpDelete();

    /// Removes the directory and all the files in it, recursively. It is used
    /// in conjunction with SetUpDelete.
    bool RemoveDir();
};

#endif
