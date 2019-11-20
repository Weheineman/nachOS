#include "filesys/FileSystemTests/fs_dir_test_Cases.hh"

void TestRootAccess(){
	if(fileSystem -> Create("/", 0)){
		printf("!!!! TestRootAccess failed: Could create a root directory");
		return;
	}
	if(fileSystem -> Remove("/")){
		printf("!!!! TestRootAccess failed: Could delete root directory");
		return;
	}	
	
	// FIND?
	
	printf("--- TestRootAccess successful!\n\n\n");
}
