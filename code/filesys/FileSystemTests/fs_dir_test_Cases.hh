// Test routines to check how nachOS handles
// access to the directory structure by many threads simultaneously.

#ifndef NACHOS_DIRECTORY_TEST_CASES__HH
#define NACHOS_DIRECTORY_TEST_CASES__HH

#include "filesys/file_system.hh"
#include "threads/system.hh"

/// Checks the expected behaviour of Add, Remove and Find operations in the root directory.
void TestRootAccess();


/// Poder crear una estructura de árbol y que las cosas se encuentren.
/// Poder eliminar cosas creadas y que ya no se encuentren.
/// Tratar de borrar directorios (vacios y no vacios).
/// Tratar de borrar directorio vacío en el que haya alguien.
/// Tratar de crear simultáneamente en distintas partes del file system.
/// Stress test multinivel?
#endif
