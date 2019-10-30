/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_FILESYS_DIRECTORY_LOCK_MANAGER_HH
#define NACHOS_FILESYS_DIRECTORY_LOCK_MANAGER_HH

#include "reader_writer.hh"

class DirectoryLockNode{
public:
    int sector;
    ReaderWriter *lock;
    DirectoryLockNode *next;

    // Amount of processes that want to take this lock.
    int waiting;
};

/// The following class defines a "directory lock manager". It is a linked
/// list containing (sector, ReaderWriter) pairs. There is a pair for each
/// directory that is being worked on, where sector is the sector number
/// in which the file header for that directory is stored, and ReaderWriter
/// is the lock for that directory.
class DirectoryLockManager {
public:
    DirectoryLockManager();

    ~DirectoryLockManager();

    void AcquireRead(int sector);

    void AcquireWrite(int sector);

    void ReleaseRead(int sector);

    void ReleaseWrite(int sector);

private:
    DirectoryLockNode *first, *last;
    Lock *managerLock;

    // If writePermission is set to true, it acquires the writer lock. Else,
    // it acquires the reader lock.
    void Acquire(int sector, bool writePermission);

    // If writePermission is set to true, it releases the writer lock. Else,
    // it releases the reader lock.
    void Release(int sector, bool writePermission);

    // Returns the node corresponding to the sector. If there is none,
    // it returns nullptr instead.
    DirectoryLockNode* FindNode(int sector);

    // Adds a node to the list with the given sector. If writePermission is
    // set to true, it acquires the writer lock. Else, it acquires the reader
    // lock.
    void AddNode(int sector, bool writePermission);

    // Removes a node from the list.
    void DeleteNode(DirectoryLockNode *target);
};


#endif
