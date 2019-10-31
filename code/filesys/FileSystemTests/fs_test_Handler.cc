#include "filesys/FileSystemTests/fs_test_Cases.hh"


void FileSystemTestHandler(){
	TestSimpleManyFiles();
	TestReadersManyFiles(1);
	TestWritersManyFiles(1);
	//~ TestReadersWriters();
	TestRemoveClosedFile();
	TestRemoveOpenFile();
	TestMultipleRemovalsWhileClosed();
	TestMultipleRemovalsWhileOpen();
	TestEditWhilePendingRemoval();
}
