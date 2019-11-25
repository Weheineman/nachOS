// Test routines to check how nachOS handles
// access to the file system by many threads simultaneously.

#ifndef NACHOS_FILESYS_TEST_CASES__HH
#define NACHOS_FILESYS_TEST_CASES__HH

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
	char fillContent;
	Semaphore *finishCheck;
	Lock *queueLock;
	Condition *queueCond;
};

struct RWWriterArg{
	char *fileName;
	unsigned writeSize, count, threadAmount, threadNum;
	Semaphore *finishCheck;
	Lock *queueLock;
	Condition *queueCond;
};

struct RWSpawnerArg{
	unsigned fileNum, writeSize, repCount, writerAmount, readerAmount;
	char fillContent;
	Semaphore *totalCheck;
};


/// Auxiliary Functions
// Creates and writes a file with the given name, its having given contents written count times.
bool WriteTestFile(char *name, char *contents, unsigned size, unsigned count);
// Reads a given content as many times as count, checking that every reading is successful.
void ReaderThread(void *threadArgs_);
// Forks a given amount of reader threads in a given file and checks that all of them finish.
void SpawnReaders(void *spawnerArgs_);
// Writes its thread ID in specific points in a given file.
void WriterThread(void *threadArgs_);
// Checks that the writers forked by SpawnWriters have written successfully in a given file.
bool CheckWriters(char *testFileName, unsigned contentSize, unsigned count, unsigned threadAmount);
// Forks a given amount of writer threads, that fill a given file collaboratively.
void SpawnWriters(void* spawnerArgs_);
// Reads a section of the given test file byte by byte. If at any moment it reads a part that
// has not been yet overwritten by a writer, it waits on a condition variable until signalled.
void RWReaderThread(void *threadArgs_);
// Writes its thread ID in specific points in a given file, similarly to Writer Thread.
// After each successful write, it broadcasts readers so that they try and start reading again.
void RWWriterThread(void *threadArgs_);
// Creates a file and forks both readers and writers to access it concurrently.
// Initially, the file consists of only hypens that are eventually overwritten by writers.
// If a reader find a hyphen, it waits on a condition variable until a writer accesses the file.
void SpawnReadersWriters(void *spawnerArgs_);


/// Test Cases
// Simple test that creates, opens and writes to two files simultaneously.
void TestSimpleManyFiles();
// Forks multiple Reader Spawners, with each of them creating a file and forking readers to access it concurrently.
void TestReadersManyFiles();
// Forks multiple Writer Spawners, with each of them creating a file and forking writers to access it concurrently.
void TestWritersManyFiles();
// Forks multiple ReaderWriter Spawners, with each of them creating a file and forking both readers and writers to access it concurrently.
void TestReadersWritersManyFiles();
// Checks that it is possible to remove a file that exists in the system but is not currently open.
void TestRemoveClosedFile();
// Checks that it is possible to remove a file that is currently open, and that it is not openable anymore after that.
void TestRemoveOpenFile();
// Checks that, if a closed file is removed multiple times, only the first one actually removes it and the rest do not find the file.
void TestMultipleRemovalsWhileClosed();
// Checks that a file can be removed any number of times while open.
void TestMultipleRemovalsWhileOpen();
// Checks that a file can be properly accessed even though it is pending to be removed.
void TestEditWhilePendingRemoval();




#endif
