// Test routines to check how nachOS handles
// access to the file system by many threads simultaneously.

#ifndef NACHOS_FILESYS_TEST_CASES__HH
#define NACHOS_FILESYS_DIRECTORY__HH

#include "filesys/file_system.hh"
#include "lib/utility.hh"
#include "machine/disk.hh"
#include "machine/statistics.hh"
#include "threads/thread.hh"
#include "threads/system.hh"

struct ReaderArg{
	char *fileName, *contents;
	unsigned contentSize, count;
	Semaphore *finishCheck;
};

struct ReaderSpawnerArg{
	char *testContents;
	unsigned fileNum, testContentSize, repCount, threadAmount;
	Semaphore *totalCheck;
};

struct WriterArg{
	char *fileName;
	unsigned writeSize, count, threadAmount, threadNum;
	Semaphore *finishCheck;
};

struct WriterSpawnerArg{
	unsigned fileNum, writeSize, repCount, threadAmount;
	Semaphore *totalCheck;
};

struct RWReaderArg{
	char *fileName;
	unsigned start, end;
	Semaphore *finishCheck;
};

/// Auxiliary Functions
bool WriteTestFile(char *name, char *contents, unsigned size, unsigned count);



/// Test Cases
void TestSimpleManyFiles();
void TestReadersManyFiles(unsigned fileAmount);
void TestWritersManyFiles(unsigned fileAmount);
//~ void TestReadersWriters(); // Falta ver de evitar el starvation de writers
void TestRemoveClosedFile();
void TestRemoveOpenFile();
void TestMultipleRemovalsWhileClosed();
void TestMultipleRemovalsWhileOpen();
void TestEditWhilePendingRemoval();




#endif
