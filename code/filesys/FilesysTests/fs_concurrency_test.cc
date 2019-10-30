// Test routines to check how nachOS handles
// access to the file system by many threads simultaneously.

#include "filesys/file_system.hh"
#include "lib/utility.hh"
#include "machine/disk.hh"
#include "machine/statistics.hh"
#include "threads/thread.hh"
#include "threads/system.hh"

// HAY QUE HACER ANDAR EL MAKEFILE, ESTRUCTURA TODO BIEN ANTES DE SEGUIR
// Grinblat propone un archivo con las definiciones comunes y dejar los tests en otro archivo.


void TestSimpleManyFiles();
void TestReadersManyFiles(unsigned fileAmount);
//~ void TestManyWriters(); // Falta hacerlo generalizado.
//~ void TestReadersWriters(); // Falta ver de evitar el starvation de writers

void FileSysConcurrencyTests(){
	//~ TestSimpleManyFiles();
	//~ TestReadersManyFiles(2);
	//~ TestManyWriters();
	//~ TestReadersWriters();
}
