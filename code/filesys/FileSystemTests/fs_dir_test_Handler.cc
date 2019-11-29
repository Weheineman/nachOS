#include "filesys/FileSystemTests/fs_dir_test_Cases.hh"

void DirectoryTestHandler(){
	// GUIDIOS: Descomentar.
	TestRootAccess();
	TestCreateDirectoryStructure();
	TestTraverseDirectoryStructure();
	TestRemoveDirectoryStructure();
	//~ TestRemoveDirectoryWithThread();
	//~ TestMultilevelStress();
}
