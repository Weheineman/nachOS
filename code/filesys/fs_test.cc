/// Simple test routines for the file system.
///
/// We implement:
///
/// Copy
///     Copy a file from UNIX to Nachos.
/// Print
///     Cat the contents of a Nachos file.
/// Perftest
///     A stress test for the Nachos file system read and write a really
///     really large file in tiny chunks (will not work on baseline system!)
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2018 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "file_system.hh"
#include "lib/utility.hh"
#include "machine/disk.hh"
#include "machine/statistics.hh"
#include "threads/thread.hh"
#include "threads/system.hh"


static const unsigned TRANSFER_SIZE = 10;  // Make it small, just to be
                                           // difficult.

/// Copy the contents of the UNIX file `from` to the Nachos file `to`.
void
Copy(const char *from, const char *to)
{
    ASSERT(from != nullptr);
    ASSERT(to != nullptr);

    // Open UNIX file.
    FILE *fp = fopen(from, "r");
    if (fp == nullptr) {
        printf("Copy: could not open input file %s\n", from);
        return;
    }

    // Figure out length of UNIX file.
    fseek(fp, 0, 2);
    int fileLength = ftell(fp);
    fseek(fp, 0, 0);

    DEBUG('f', "Copying file %s, size %u, to file %s\n",
          from, fileLength, to);

    // Create a Nachos file of the same length.
    if (!fileSystem->Create(to, fileLength)) {  // Create Nachos file.
        printf("Copy: could not create output file %s\n", to);
        fclose(fp);
        return;
    }

    OpenFile *openFile = fileSystem->Open(to);
    ASSERT(openFile != nullptr);

    // Copy the data in `TRANSFER_SIZE` chunks.
    char *buffer = new char [TRANSFER_SIZE];
    int amountRead;
    while ((amountRead = fread(buffer, sizeof(char),
                               TRANSFER_SIZE, fp)) > 0)
        openFile->Write(buffer, amountRead);
    delete [] buffer;

    // Close the UNIX and the Nachos files.
    delete openFile;
    fclose(fp);
}

/// Print the contents of the Nachos file `name`.
void
Print(const char *name)
{
    ASSERT(name != nullptr);

    OpenFile *openFile = fileSystem->Open(name);
    if (openFile == nullptr) {
        fprintf(stderr, "Print: unable to open file %s\n", name);
        return;
    }

    char *buffer = new char [TRANSFER_SIZE];
    int amountRead;
    while ((amountRead = openFile->Read(buffer, TRANSFER_SIZE)) > 0)
        for (unsigned i = 0; i < (unsigned) amountRead; i++)
            printf("%c", buffer[i]);

    delete [] buffer;
    delete openFile;  // close the Nachos file
}


/// Performance test
///
/// Stress the Nachos file system by creating a large file, writing it out a
/// bit at a time, reading it back a bit at a time, and then deleting the
/// file.
///
/// Implemented as three separate routines:
/// * `FileWrite` -- write the file.
/// * `FileRead` -- read the file.
/// * `PerformanceTest` -- overall control, and print out performance #'s.

//static const char FILE_NAME[] = "TestFile2";
static const char CONTENTS[] = "1234567890";
static const unsigned CONTENT_SIZE = sizeof CONTENTS - 1;
static const unsigned FILE_SIZE = CONTENT_SIZE * 1000;

static void
FileWrite(const char FILE_NAME[])
{
    printf("Sequential write of %u byte file, in %u byte chunks\n",
           FILE_SIZE, CONTENT_SIZE);

    if (!fileSystem->Create(FILE_NAME, FILE_SIZE / 2 + 1647)) {
        fprintf(stderr, "Perf test: cannot create %s\n", FILE_NAME);
        return;
    }
    printf("Creation successful\n");
    OpenFile *openFile = fileSystem->Open(FILE_NAME);
    if (openFile == nullptr) {
        fprintf(stderr, "Perf test: unable to open %s\n", FILE_NAME);
        return;
    }
    printf("Opening successful\n");
    for (unsigned i = 0; i < FILE_SIZE; i += CONTENT_SIZE) {
        unsigned numBytes = openFile->Write(CONTENTS, CONTENT_SIZE);

        if (numBytes < CONTENT_SIZE) {
            fprintf(stderr, "Perf test: unable to write %s\n", FILE_NAME);
            break;
        }
    }
    printf("Writing successful\n");
    delete openFile;
}

static void
FileRead(const char FILE_NAME[])
{
    printf("Sequential read of %u byte file, in %u byte chunks\n",
           FILE_SIZE, CONTENT_SIZE);

    OpenFile *openFile = fileSystem->Open(FILE_NAME);
    if (openFile == nullptr) {
        fprintf(stderr, "Perf test: unable to open file %s\n", FILE_NAME);
        return;
    }

    char *buffer = new char [CONTENT_SIZE];
    for (unsigned i = 0; i < FILE_SIZE; i += CONTENT_SIZE) {
        unsigned numBytes = openFile->Read(buffer, CONTENT_SIZE);
        if (numBytes < CONTENT_SIZE || strncmp(buffer, CONTENTS, CONTENT_SIZE)) {
            printf("Perf test: unable to read %s\n", FILE_NAME);
            break;
        }
    }
    printf("Reading succesful\n");
    delete [] buffer;
    delete openFile;
}

void
PerformanceTest()
{
    printf("Starting file system performance test:\n");
    //stats->Print();
    const char Name1[] = "TestFile";
    FileWrite(Name1);
    FileRead(Name1);
     if (!fileSystem->Remove(Name1)) {
        printf("Perf test: unable to remove %s\n", Name1);
        return;
    }
    //stats->Print();
}
