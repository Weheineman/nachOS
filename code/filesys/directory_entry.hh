/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_FILESYS_DIRECTORYENTRY__HH
#define NACHOS_FILESYS_DIRECTORYENTRY__HH

#include "open_file.hh"

/// The following class defines a "directory entry", representing a file in
/// the directory.  Each entry gives the name of the file, and where the
/// file's header is to be found on disk.
///
/// Internal data structures kept public so that Directory operations can
/// access them directly.
class DirectoryEntry {
public:
    DirectoryEntry(unsigned _sector, bool _isDirectory, const char * _name){
        memset(this, 0, sizeof(DirectoryEntry));
        sector = _sector;
        isDirectory = _isDirectory;
        strncpy(name, _name, FILE_NAME_MAX_LEN);
        next = nullptr;
    }

    /// Is this entry a Directory?
    bool isDirectory;
    /// Location on disk to find the `FileHeader` for this file.
    unsigned sector;
    /// Pointer to the next DirectoryEntry
    DirectoryEntry *next;
    /// Text name for file, with +1 for the trailing `'\0'`.
    //GUIDIOS: Le tuve que sacar el +1 por el tema del padding. Ver qué hacemos con esto.
    char name[FILE_NAME_MAX_LEN];

};


#endif
