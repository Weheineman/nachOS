// Test routines to check how nachOS handles
// access to the directory structure by many threads simultaneously.

#ifndef NACHOS_DIRECTORY_TEST_CASES__HH
#define NACHOS_DIRECTORY_TEST_CASES__HH

#include <cstdlib>
#include "filesys/file_system.hh"
#include "threads/system.hh"

struct RemoveDirChildArg{
	Semaphore* parentReady, *childReady;
	char *subDirectory;
};

struct MultiLevelStressArg{
	char *path;
	char *toWrite;
	unsigned writeAmount, writeSize, fileAmount;
	Semaphore *finishCheck;	
};


/// Auxiliary Functions
// Child thread used in TestRemoveDirectoryWithThread.
void RemoveDirChild(void *args_);
// Thread that creates and writes to many files in the path given to it.
// Used in TestMultilevelStress.
void MultilevelStressThread(void *args_);


/// Test Cases
// Checks the expected behaviour of Add, Remove and Find operations in the root directory.
void TestRootAccess();
// Checks that a simple directory structure can be created, using both relative and global paths.
// Also checks that duplicate creations are not possible.
void TestCreateDirectoryStructure();
// Checks that the structure previously created is correctly traversable.
void TestTraverseDirectoryStructure();
// Checks that both files and empty directories can be removed.
// Also checks that non existing files and populated directories cannot be removed.
void TestRemoveDirectoryStructure();
// Checks that an empty directory with a thread located inside it is still removable.
void TestRemoveDirectoryWithThread();
// Creates a directory structure and forks threads to each of them to create
// files and write to them concurrently.
void TestMultilevelStress();
#endif
