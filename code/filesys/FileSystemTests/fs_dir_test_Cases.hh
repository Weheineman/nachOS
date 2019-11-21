// Test routines to check how nachOS handles
// access to the directory structure by many threads simultaneously.

#ifndef NACHOS_DIRECTORY_TEST_CASES__HH
#define NACHOS_DIRECTORY_TEST_CASES__HH

#include "filesys/file_system.hh"
#include "threads/system.hh"

/// Checks the expected behaviour of Add, Remove and Find operations in the root directory.
void TestRootAccess();
/// Checks that a simple directory structure can be created, using both relative and global paths.
/// Also checks that duplicate creations are not possible.
void TestCreateDirectoryStructure();
/// Checks that the structure previously created is correctly traversable.
void TestTraverseDirectoryStructure();
/// Checks that both files and empty directories can be removed.
/// Also checks that non existing files and populated directories cannot be removed.
void TestRemoveDirectoryStructure();


/// Tratar de borrar directorio vacío en el que haya alguien.
/// Tratar de crear simultáneamente en distintas partes del file system.
/// Stress test multinivel?
#endif
