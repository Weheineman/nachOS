#include "threads/synch.hh"

struct FileMetadataNode{
    // Name of the file
    char *name;
    // Used to control concurrent access
    Lock *lock;
    // Amount of OpenFile instances that reference the file
    int openInstances;
    // True iff Remove has been called on the file
    bool pendingRemove;

    FileMetadataNode *next;
};

class OpenFileList {
    public:
        OpenFileList();

        ~OpenFileList();

        // Adds the file to the open file list.
        // If the file is already open:
        //      and is pending removal, it does nothing.
        //      else it increases openInstances by 1.
        void AddOpenFile(const char *fileName);

        // Decreases the openInstances by 1. If no open instances remain,
        // the file is removed from the list.
        void CloseOpenFile(const char *fileName);

        void DeleteOpenFile(const char *fileName);

    private:
        FileMetadataNode* FindOpenFile(const char *fileName);

        FileMetadataNode *first, *last;
        Lock *listLock;
};
