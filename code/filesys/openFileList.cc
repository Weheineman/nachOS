#include "openFileList.hh"

OpenFileList::OpenFileList()
{
    listLock = new Lock;
    first = last = nullptr;
}

OpenFileList::~OpenFileList()
{
    // DELETE ELEMENTS
    delete listLock;
}

// Adds the file to the open file list.
// If the file is already open:
//      and is pending removal, it does nothing.
//      else it increases openInstances by 1.
void
OpenFileList::AddOpenFile(const char *fileName_){

}
