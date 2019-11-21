#include "filesys/FileSystemTests/fs_dir_test_Cases.hh"

/// Checks the expected behaviour of Add, Remove and Find operations in the root directory.
void TestRootAccess(){
	if(fileSystem -> Create("/", 0) or fileSystem -> Create("", 0)){
		printf("!!!! TestRootAccess failed: Could create a root directory\n");
		return;
	}
	if(fileSystem -> Remove("/") or fileSystem -> Remove("")){
		printf("!!!! TestRootAccess failed: Could delete root directory\n");
		return;
	}

	if(not fileSystem -> ChangeDirectory("/") or not fileSystem -> ChangeDirectory("")){
		printf("!!!! TestRootAccess failed: Could not move to root directory\n");
		return;
	}

	printf("--- TestRootAccess successful!\n\n\n");
}


/// Checks that a simple directory structure can be created, using both relative and global paths.
/// Also checks that duplicate creations are not possible.
void TestCreateDirectoryStructure(){
	if(not fileSystem -> Create("/1", 0, true) or
	   not fileSystem -> Create("/2", 0, true) or
	   not fileSystem -> Create("/3", 0, false)){
		printf("!!!! TestCreateDirectoryStructure failed: Could not create files in root folder\n");
		return;
	}

	if(fileSystem -> Create("/1", 0, true) or
	   fileSystem -> Create("/1", 0, false) or
	   fileSystem -> Create("/2", 0, true) or
	   fileSystem -> Create("/2", 0, false) or
	   fileSystem -> Create("/3", 0, true) or
	   fileSystem -> Create("/3", 0, false)){
		printf("!!!! TestCreateDirectoryStructure failed: Could create a file twice in root folder\n");
		return;
	}

	if(fileSystem -> Create("/3/Fail", 0, true) or
	   fileSystem -> Create("/3/Fail", 0, false)){
		printf("!!!! TestCreateDirectoryStructure failed: Could create a file inside a file in root folder\n");
		return;
	}

	if(fileSystem -> Create("/NonExisting/Fail", 0, true) or
	   fileSystem -> Create("/NonExisting/Fail", 0, false)){
		printf("!!!! TestCreateDirectoryStructure failed: Could create a file with a non existing global path\n");
		return;
	}

	if(not fileSystem -> Create("/1/1A", 0, true) or
	   not fileSystem -> Create("/1/1B", 0, false)){
		printf("!!!! TestCreateDirectoryStructure failed: Could not create a file using a global path\n");
		return;
	}

	if(fileSystem -> Create("/1/1A", 0, true) or
	   fileSystem -> Create("/1/1A", 0, false) or
	   fileSystem -> Create("/1/1B", 0, true) or
	   fileSystem -> Create("/1/1B", 0, false)){
		printf("!!!! TestCreateDirectoryStructure failed: Could create a file twice using a global path\n");
		return;
	}

	if(fileSystem -> Create("/1/1B/Fail", 0, true) or
	   fileSystem -> Create("/1/1B/Fail", 0, false)){
		printf("!!!! TestCreateDirectoryStructure failed: Could create a file inside a file using a global path\n");
		return;
	}

	if(not fileSystem -> ChangeDirectory("/2")){
		printf("!!!! TestCreateDirectoryStructure failed: Could not move to a subdirectory\n");
		return;
	}

	if(not fileSystem -> Create("2A", 0, true) or
	   not fileSystem -> Create("2B", 0, false)){
		printf("!!!! TestCreateDirectoryStructure failed: Could not create a file using a relative path\n");
		return;
	}

	if(fileSystem -> Create("2A", 0, true) or
	   fileSystem -> Create("2A", 0, false) or
	   fileSystem -> Create("2B", 0, true) or
	   fileSystem -> Create("2B", 0, false)){
		printf("!!!! TestCreateDirectoryStructure failed: Could create a file twice using a relative path\n");
		return;
	}

	if(fileSystem -> Create("2B/Fail", 0, true) or
	   fileSystem -> Create("2B/Fail", 0, false)){
		printf("!!!! TestCreateDirectoryStructure failed: Could create a file inside a file using a relative path\n");
		return;
	}

	if(fileSystem -> Create("NonExisting/Fail", 0, true) or
	   fileSystem -> Create("NonExisting/Fail", 0, false)){
		printf("!!!! TestCreateDirectoryStructure failed: Could create a file with a non existing relative path\n");
		return;
	}

	if(not fileSystem -> ChangeDirectory("..")){
		printf("!!!! TestCreateDirectoryStructure failed: Could not move back to root\n");
		return;
	}

	printf("--- TestCreateDirectoryStructure successful!\n\n\n");
}


/// Checks that the structure previously created is correctly traversable.
void TestTraverseDirectoryStructure(){
	if(not fileSystem -> ChangeDirectory("/1") or
	   not fileSystem -> ChangeDirectory("/2") or
	   not fileSystem -> ChangeDirectory("/1/1A") or
	   not fileSystem -> ChangeDirectory("/2/2A") or
	   not fileSystem -> ChangeDirectory("/")){
		printf("!!!! TestTraverseDirectoryStructure failed: Could not move to a directory with a global path\n");
		return;
	}

	if(fileSystem -> ChangeDirectory("/1/1B") or
	   fileSystem -> ChangeDirectory("/2/2B")){
		printf("!!!! TestTraverseDirectoryStructure failed: Could move to a file with a global path\n");
		return;
	}

	if(fileSystem -> ChangeDirectory("3")){
		printf("!!!! TestTraverseDirectoryStructure failed: Could move to a root file with a relative path\n");
		return;
	}

	if(fileSystem -> ChangeDirectory("/NonExisting")){
		printf("!!!! TestTraverseDirectoryStructure failed: Could move to a non existing directory with a global path\n");
		return;
	}

	if(not fileSystem -> ChangeDirectory("1") or
	   not fileSystem -> ChangeDirectory("1A")){
		printf("!!!! TestTraverseDirectoryStructure failed: Could not move to a directory with a relative path\n");
		return;
	}

	if(fileSystem -> ChangeDirectory("NonExisting")){
		printf("!!!! TestTraverseDirectoryStructure failed: Could move to a non existing directory with a relative path\n");
		return;
	}

	if(not fileSystem -> ChangeDirectory("..") or
	   not fileSystem -> ChangeDirectory("..")){
		printf("!!!! TestTraverseDirectoryStructure failed: Could not move back to root directory using ..\n");
		return;
	}

	if(not fileSystem -> ChangeDirectory(".")){
		printf("!!!! TestTraverseDirectoryStructure failed: Could not move to the same directory using .\n");
		return;
	}

	printf("--- TestTraverseDirectoryStructure successful!\n\n\n");
}


/// Checks that both files and empty directories can be removed.
/// Also checks that non existing files and populated directories cannot be removed.
void TestRemoveDirectoryStructure(){
	if(fileSystem -> Remove("/NonExisting")){
		printf("!!!! TestRemoveDirectoryStructure failed: Could remove a nonexisting file using a global path\n");
		return;
	}

	if(fileSystem -> Remove("NonExisting")){
		printf("!!!! TestRemoveDirectoryStructure failed: Could remove a nonexisting file using a relative path\n");
		return;
	}

	if(fileSystem -> Remove("/1")){
		printf("!!!! TestRemoveDirectoryStructure failed: Could remove a populated directory using a global path\n");
		return;
	}

	if(not fileSystem -> Remove("/1/1A") or
	   not fileSystem -> Remove("/1/1B")){
		printf("!!!! TestRemoveDirectoryStructure failed: Could not remove a file using a global path\n");
		return;
	}

	if(fileSystem -> ChangeDirectory("/1/1A")){
		printf("!!!! TestRemoveDirectoryStructure failed: Could move to a directory previously removed using a global path\n");
		return;
	}

	if(not fileSystem -> Remove("/1")){
		printf("!!!! TestRemoveDirectoryStructure failed: Could not remove a root subdirectory using a global path\n");
		return;
	}

	if(fileSystem -> ChangeDirectory("/1")){
		printf("!!!! TestRemoveDirectoryStructure failed: Could move to a root subdirectory previously removed using a global path\n");
		return;
	}

	if(fileSystem -> Remove("2")){
		printf("!!!! TestRemoveDirectoryStructure failed: Could remove a populated directory using a relative path\n");
		return;
	}

	if(not fileSystem -> ChangeDirectory("2")){
		printf("!!!! TestRemoveDirectoryStructure failed: Could not move to a root subdirectory\n");
		return;
	}

	if(not fileSystem -> Remove("2A") or
	   not fileSystem -> Remove("2B")){
		printf("!!!! TestRemoveDirectoryStructure failed: Could not remove a file using a relative path\n");
		return;
	}

	if(fileSystem -> ChangeDirectory("2A")){
		printf("!!!! TestRemoveDirectoryStructure failed: Could move to a directory previously removed using a relative path\n");
		return;
	}

	if(not fileSystem -> ChangeDirectory("..")){
		printf("!!!! TestRemoveDirectoryStructure failed: Could not move back to root\n");
		return;
	}

	if(not fileSystem -> Remove("2")){
		printf("!!!! TestRemoveDirectoryStructure failed: Could not remove a root subdirectory using a relative path\n");
		return;
	}

	if(fileSystem -> ChangeDirectory("2")){
		printf("!!!! TestRemoveDirectoryStructure failed: Could move to a root subdirectory previously removed using a relative path\n");
		return;
	}

	if(not fileSystem -> Remove("3")){
		printf("!!!! TestRemoveDirectoryStructure failed: Could not remove a root file using a relative path\n");
		return;
	}

	printf("--- TestRemoveDirectoryStructure successful!\n\n\n");
}
