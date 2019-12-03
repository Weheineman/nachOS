#include "filesys/FileSystemTests/fs_dir_test_Cases.hh"

// Checks the expected behaviour of Add, Remove and Find operations in the root directory.
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


// Checks that a simple directory structure can be created, using both relative and global paths.
// Also checks that duplicate creations are not possible.
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


// Checks that the structure previously created is correctly traversable.
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


// Checks that both files and empty directories can be removed.
// Also checks that non existing files and populated directories cannot be removed.
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


// Child thread used in TestRemoveDirectoryWithThread.
void RemoveDirChild(void *args_){
	RemoveDirChildArg *args = (RemoveDirChildArg *) args_;
	Semaphore *parentReady = args -> parentReady;
	Semaphore *childReady = args -> childReady;
	char *subDirectory = args -> subDirectory;

	if(not fileSystem -> ChangeDirectory(subDirectory)){
		printf("!!!! TestRemoveDirectoryWithThread failed: Child thread could not move to test directory\n");
		return;
	}

	childReady -> V();
	parentReady -> P();

	if(not fileSystem -> ChangeDirectory("..")){
		printf("!!!! TestRemoveDirectoryWithThread failed: Child thread could not move to its parent directory\n");
		return;
	}

	childReady -> V();
}


// Checks that an empty directory with a thread located inside it is still removable.
void TestRemoveDirectoryWithThread(){
	char subDirectory[] = "/Test";

	if(not fileSystem -> Create(subDirectory, 0, true)){
		printf("!!!! TestRemoveDirectoryWithThread failed: Could not create test directory\n");
		return;
	}

	Semaphore *parentReady = new Semaphore("Parent Directory Ready", 0);
	Semaphore *childReady  = new Semaphore("Child Directory Ready", 0);

	RemoveDirChildArg *childArgs = new RemoveDirChildArg;
	childArgs -> parentReady = parentReady;
	childArgs -> childReady = childReady;
	childArgs -> subDirectory = subDirectory;

	Thread *childThread = new Thread("Child Thread");
	childThread -> Fork(RemoveDirChild, (void *) childArgs);

	childReady -> P();

	if(not fileSystem -> Remove(subDirectory)){
		printf("!!!! TestRemoveDirectoryWithThread failed: Could not remove test directory with child thread inside\n");
		return;
	}

	parentReady -> V();
	childReady -> P();

	delete parentReady;
	delete childReady;
	delete childArgs;

	printf("--- TestRemoveDirectoryWithThread successful!\n\n\n");
}


// Thread that creates and writes to many files in the path given to it.
// Used in TestMultilevelStress.
void MultilevelStressThread(void *args_){
	MultiLevelStressArg *args = (MultiLevelStressArg *) args_;
	char *path = args -> path;
	char *toWrite = args -> toWrite;
	unsigned writeAmount = args -> writeAmount;
	unsigned writeSize = args -> writeSize;
	unsigned fileAmount = args -> fileAmount;
	Semaphore *finishCheck = args -> finishCheck;

	/// Move to the given subdirectory.
	if(not fileSystem -> ChangeDirectory(path)){
		printf("!!!! TestMultilevelStress failed: Child could not move to directory %s\n", path);
		return;
	}

	/// Create each file.
	char fileName[10];
	for(unsigned i = 0; i < fileAmount; i++){
		snprintf(fileName, 10, "%d", i);
		if(not fileSystem -> Create(fileName, 0, false)){
			printf("!!!! TestMultilevelStress failed: Child could not create file %d in directory %s\n", i, path);
			return;
		}
	}

	/// Open each file.
	OpenFile *descriptors[fileAmount];
	for(unsigned i = 0; i < fileAmount; i++){
		snprintf(fileName, 10, "%d", i);
		descriptors[i] = fileSystem -> Open(fileName);
		if(descriptors[i] == nullptr){
			printf("!!!! TestMultilevelStress failed: Child could not open file %d in directory %s\n", i, path);
			return;
		}
	}

	/// Write to each file.
	for(unsigned writeNum = 0; writeNum < writeAmount; writeNum ++){
		for(unsigned file = 0; file < fileAmount; file ++){
			unsigned numBytes = descriptors[file] -> Write(toWrite, writeSize);
			if (numBytes < writeSize) {
				printf("!!!! TestMultilevelStress failed: Child could not write to file %d on iteration %d in directory %s\n", file, writeNum, path);
				return;
			}
		}
	}

	// Move back the seek position to the beginning of the file.
	for(unsigned file = 0; file < fileAmount; file++)
		descriptors[file] -> Seek(0);

	/// Read each file and check it was correctly written.
	char buffer[writeSize + 1];
	for(unsigned readNum = 0; readNum < writeAmount; readNum ++){
		for(unsigned file = 0; file < fileAmount; file ++){
			unsigned numBytes = descriptors[file] -> Read(buffer, writeSize);
			if (numBytes < writeSize or strncmp(toWrite, buffer, writeSize)) {
				printf("!!!! TestMultilevelStress failed: Child could not read file %d on iteration %d in directory %s\n", file, readNum, path);
				return;
			}
		}
	}

	/// Close each file.
	for(unsigned i = 0; i < fileAmount; i++)
		delete descriptors[i];

	/// Remove each file.
	for(unsigned i = 0; i < fileAmount; i++){
		snprintf(fileName, 10, "%d", i);
		if(not fileSystem -> Remove(fileName)){
			printf("!!!! TestMultilevelStress failed: Child could not remove file %d in directory %s\n", i, path);
			return;
		}
	}

	/// Report to master thread.
	finishCheck -> V();
}

// Creates a directory structure and forks threads to each of them to create
// files and write to them concurrently.
void TestMultilevelStress(){
	const unsigned subDirLen = 10;
	char subDirs[][subDirLen] = {"/", "/A", "/B", "/A/A", "/A/B", "/B/A", "/B/B"};
	unsigned subDirCount = (sizeof subDirs) / subDirLen;

	// Setting i initially to 1 on purpose to ignore the root level.
	for(unsigned i = 1; i < subDirCount; i++)
		if(not fileSystem -> Create(subDirs[i], 0, true)){
			printf("!!!! TestMultilevelStress failed: Could not create directory %s\n", subDirs[i]);
			return;
		}

	char toWrite[] = "1234567890";
	unsigned writeAmount = 100;
	unsigned writeSize = sizeof toWrite;
	unsigned fileAmount = 5;
	Semaphore *finishCheck = new Semaphore("Multilevel Stress Test", 0);

	MultiLevelStressArg *threadArgs = new MultiLevelStressArg[subDirCount];
	for(unsigned i = 0; i < subDirCount; i++){
		threadArgs[i].path = subDirs[i];
		threadArgs[i].toWrite = toWrite;
		threadArgs[i].writeAmount = writeAmount;
		threadArgs[i].writeSize = writeSize;
		threadArgs[i].fileAmount = fileAmount;
		threadArgs[i].finishCheck = finishCheck;
		Thread *newThread = new Thread("Multilevel Stress Thread");
		newThread -> Fork(MultilevelStressThread, (void *) (threadArgs + i));
	}

	for(unsigned i = 0; i < subDirCount; i++)
		finishCheck -> P();

	for(unsigned i = subDirCount - 1; i >= 1; i --)
		if(not fileSystem -> Remove(subDirs[i]))
			printf("!!!! TestMultilevelStress failed, kinda: Every child finished executing but could not remove directory %s\n", subDirs[i]);

	delete finishCheck;

	printf("--- TestMultilevelStress successful!\n\n\n");
}
