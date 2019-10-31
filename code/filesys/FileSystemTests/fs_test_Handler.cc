#include "filesys/FileSystemTests/fs_test_Cases.hh"

void FileSystemTestHandler(){
	TestSimpleManyFiles();
	TestReadersManyFiles();
	TestWritersManyFiles();
	TestReadersWritersManyFiles();
	TestRemoveClosedFile();
	TestRemoveOpenFile();
	TestMultipleRemovalsWhileClosed();
	TestMultipleRemovalsWhileOpen();
	TestEditWhilePendingRemoval();
}
