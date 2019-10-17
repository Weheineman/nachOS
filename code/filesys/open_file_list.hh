#ifndef NACHOS_FILESYS_OPENFILE_LIST__HH
#define NACHOS_FILESYS_OPENFILE_LIST__HH

#include "reader_writer.hh"
#include "threads/synch.hh"
#include "file_system.hh"

class Lock;
class ReaderWriter;
class FileSystem;


struct FileMetadataNode{
    // Name of the file
    char *name;
    // Used to control concurrent access
    ReaderWriter *lock;
    // Amount of OpenFile instances that reference the file
    int openInstances;
    // True iff Remove has been called on the file
    bool pendingRemove;

    FileMetadataNode *next;
};

// All public methods of the OpenFileList class are atomic.
class OpenFileList {
    public:
        OpenFileList(FileSystem *);

        ~OpenFileList();

        // Adds the file to the open file list.
        // If the file is already open:
        //      and is pending removal, it does nothing.
        //      else it increases openInstances by 1.
        ReaderWriter* AddOpenFile(const char *fileName);

        // Decreases the openInstances by 1. If no open instances remain,
        // the file is removed from the list.
        void CloseOpenFile(const char *fileName);

        // Returns true if the file is currently open, in which case
        // SetUpRemoval sets pendingRemove to true atomically.
        // If the file is not open, it just returns false.
        // Assumes the fileListLock is already taken by the file system.
        bool SetUpRemoval(const char *fileName);
        
        // Allows the file system to acquire the list's lock.
        void AcquireListLock();
        
        // Allows the file system to release the list's lock.
        void ReleaseListLock();
        

    private:
        FileMetadataNode* FindOpenFile(const char *fileName);
        bool IsEmpty();
        FileMetadataNode* CreateNode (const char *fileName);
        void DeleteNode(FileMetadataNode *target);

        Lock *listLock;
        FileMetadataNode *first, *last;
        
        FileSystem* myFileSystem;
};


#endif
