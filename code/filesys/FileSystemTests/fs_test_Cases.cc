#include "filesys/FileSystemTests/fs_test_Cases.hh"

// Simple test that creates, opens and writes to two files simultaneously.
void TestSimpleManyFiles(){
	char file1[] = "Test 1";
	char file2[] = "Test 2";
	
	unsigned count = 100;
	char contents[] = "12345";
	unsigned size = sizeof contents - 1;
	
	if(not fileSystem -> Create(file1, 0)){
		printf("Cannot create test file %s\n", file1);
		return;
	}
	OpenFile *openFile1 = fileSystem->Open(file1);
    if(openFile1 == nullptr) {
        printf("Unable to open test file %s\n", file1);
        return;
    }
    
	
	if(not fileSystem -> Create(file2, 0)){
		printf("Cannot create test file %s\n", file2);
		return;
	}
	OpenFile *openFile2 = fileSystem->Open(file2);
    if(openFile2 == nullptr) {
        printf("Unable to open test file %s\n", file2);
        return;
    }

	unsigned i;
	for (i = 0; i < count; i++) {
		unsigned numBytes = openFile1->Write(contents, size);
		if (numBytes < size) {
			printf("Unable to write on test file %s on iteration %d\n", file1, i);
			break;
		}
		numBytes = openFile2->Write(contents, size);
		if (numBytes < size) {
			printf("Unable to write on test file %s on iteration %d\n", file2, i);
			break;
		}
	}

	if(not fileSystem -> Remove(file1))
		printf("Unable to remove test file %s\n", file1);
	
	else if(not fileSystem -> Remove(file2))
		printf("Unable to remove test file %s\n", file2);

	else{
		delete openFile1;
		delete openFile2;
		
		if(i == count)
			printf("--- TestSimpleManyFiles successful!\n\n\n");
		else
			printf("!!!! TestSimpleManyFiles unsuccessful: Writers failed to write correctly.\n\n\n");
	}
}


// Creates and writes a file with the given name, its having given contents written count times.
bool WriteTestFile(char *name, char *contents, unsigned size, unsigned count){
	if(not fileSystem->Create(name, size * count)) {
        printf("Cannot create test file %s\n", name);
        return false;
    }

    OpenFile *openFile = fileSystem->Open(name);
    if(openFile == nullptr) {
        printf("Unable to open test file %s\n", name);
        return false;
    }

	unsigned i;
	for (i = 0; i < count; i++) {
		unsigned numBytes = openFile->Write(contents, size);

		if (numBytes < size) {
			printf("Unable to write on test file %s\n", name);
			break;
		}
	}

	delete openFile;
    return i == count;
}

// Reads a given content as many times as count, checking that every reading is successful.
void ReaderThread(void *threadArgs_){
	ReaderArg *threadArgs = (ReaderArg *) threadArgs_;

	char* fileName = threadArgs -> fileName;
	char* contents = threadArgs -> contents;
	unsigned contentSize = threadArgs -> contentSize;
	unsigned count = threadArgs -> count;
	Semaphore *finishCheck = threadArgs -> finishCheck;


	OpenFile *openFile = fileSystem->Open(fileName);
    if (openFile == nullptr) {
        printf("Reader %s was unable to open test file %s\n", currentThread -> GetName(), fileName);
        return;
    }

	char* buffer = new char[contentSize];
	unsigned read;
	for(read = 0; read < count; read++){
		unsigned numBytes = openFile -> Read(buffer, contentSize);
        if (numBytes < contentSize || strncmp(buffer, contents, contentSize)) {
            printf("Reader %s failed to read test file %s on iteration %d\n", currentThread -> GetName(), fileName, read);
            break;
        }
	}

	delete [] buffer;
	delete openFile;

	if(read == count){
		//~ printf("Reader %s finished reading successfully!\n", currentThread -> GetName());
		finishCheck -> V();
	}
}

// Forks a given amount of reader threads in a given file and checks that all of them finish.
void SpawnReaders(void *spawnerArgs_){
	ReaderSpawnerArg *spawnerArgs = (ReaderSpawnerArg *) spawnerArgs_;
	char *testContents = spawnerArgs -> testContents;
	unsigned fileNum = spawnerArgs -> fileNum;
	unsigned testContentSize = spawnerArgs -> testContentSize;
	unsigned repCount = spawnerArgs -> repCount;
	unsigned threadAmount = spawnerArgs -> threadAmount;
	Semaphore *totalCheck = spawnerArgs -> totalCheck;
	
	char *testFileName = new char[64];
	snprintf(testFileName, 64, "%s%d", "ManyReaders ", fileNum);

	if(not WriteTestFile(testFileName, testContents, testContentSize, repCount)){
		printf("Failed to create test file %s\n", testFileName);
		return;
	}
	
	Semaphore* finishCheck = new Semaphore("SpawnReaders", 0);
	ReaderArg* threadArgs = new ReaderArg;
	threadArgs -> fileName = testFileName;
	threadArgs -> contents = testContents;
	threadArgs -> contentSize = testContentSize;
	threadArgs -> count = repCount;
	threadArgs -> finishCheck = finishCheck;

	char *threadName = new char [64];
	for(unsigned threadNum = 0; threadNum < threadAmount; threadNum ++){
		snprintf(threadName, 64, "%s%d%s%d", "File ", fileNum, " Number ", threadNum);
		Thread *newThread = new Thread(threadName);
		newThread->Fork(ReaderThread, (void*) threadArgs);
	}

	for(unsigned i = 0; i < threadAmount; i++)
		finishCheck -> P();

	if (not fileSystem->Remove(testFileName))
        printf("Test finished but failed to remove test file %s\n", testFileName);

    delete threadArgs;
    delete [] testFileName;
    delete [] threadName;
    delete finishCheck;
    
    totalCheck -> V();
}

// Forks multiple Reader Spawners, with each of them creating a file and forking readers to access it concurrently.
void TestReadersManyFiles(){
	unsigned fileAmount = 1;
	char testContents[] = "1234567890";
	unsigned testContentSize = sizeof testContents - 1;
	unsigned repCount = 100;
	unsigned threadAmount = 3;

	ReaderSpawnerArg *args = new ReaderSpawnerArg[fileAmount];
	char *spawnerName = new char[64];
	Semaphore *totalCheck = new Semaphore("TestReadersManyFiles", 0);
	for(unsigned fileNum = 0; fileNum < fileAmount; fileNum++){
		args[fileNum].testContents = testContents;
		args[fileNum].fileNum = fileNum;
		args[fileNum].testContentSize = testContentSize;
		args[fileNum].repCount = repCount;
		args[fileNum].threadAmount = threadAmount;
		args[fileNum].totalCheck = totalCheck;
		
		snprintf(spawnerName, 64, "%s%d", "Spawner ", fileNum);
		Thread *newThread = new Thread(spawnerName);
		newThread->Fork(SpawnReaders, (void*) (args + fileNum));		
	}
	
	for(unsigned i = 0; i < fileAmount; i++)
		totalCheck -> P();
	
	delete [] args;
	delete [] spawnerName;
	delete totalCheck;
	
	printf("--- TestReadersManyFiles successful!\n\n\n");
}


// Writes its thread ID in specific points in a given file.
void WriterThread(void *threadArgs_){
	WriterArg *threadArgs = (WriterArg *) threadArgs_;

	char* fileName = threadArgs -> fileName;
	unsigned writeSize = threadArgs -> writeSize;
	unsigned count = threadArgs -> count;
	unsigned threadAmount = threadArgs -> threadAmount;
	unsigned threadNum = threadArgs -> threadNum;
	Semaphore *finishCheck = threadArgs -> finishCheck;


	OpenFile *openFile = fileSystem->Open(fileName);
    if (openFile == nullptr) {
        printf("Writer %s was unable to open test file %s\n", currentThread -> GetName(), fileName);
        return;
    }

	// Fills a buffer with the thread number at the right end of it, 
	// filling any spaces to the left with zeroes.
	char* buffer = new char[writeSize + 1];
	snprintf(buffer, writeSize, "%d", threadNum);
	unsigned idLength = strlen(buffer);
	unsigned lenDiff = writeSize - idLength;
	
	for(unsigned i = 0; i < idLength; i++)
		buffer[i + lenDiff] = buffer[i];
		
	for(unsigned i = 0; i < lenDiff; i++)
		buffer[i] = '0';
		
	buffer[writeSize] = '\0';


	unsigned write;
	for(write = 0; write < count; write++){
		unsigned offset = writeSize * threadNum + write * writeSize * threadAmount;
		unsigned numBytes = openFile -> WriteAt(buffer, writeSize, offset);
        if (numBytes < writeSize) {
            printf("Writer %s failed to write test file %s on iteration %d\n", currentThread -> GetName(), fileName, write);
            break;
        }
	}

	delete [] buffer;
	delete openFile;

	if(write == count){
		//~ printf("Writer %s finished writing successfully!\n", currentThread -> GetName());
		finishCheck -> V();
	}
}

// Checks that the writers forked by SpawnWriters have written successfully in a given file.
bool CheckWriters(char *testFileName, unsigned contentSize, unsigned count, unsigned threadAmount){
	OpenFile *openFile = fileSystem->Open(testFileName);
	if(openFile == nullptr) {
		printf("Checker was unable to open test file %s\n", testFileName);
		return false;
	}

	char *buffer = new char[contentSize + 1];
	unsigned read;
	for(read = 0; read < count * threadAmount; read++){
		unsigned numBytes = openFile -> Read(buffer, contentSize);
		if(numBytes < contentSize){
			printf("Checker failed to read test file %s on iteration %d\n", testFileName, read);
			printf("Expected read size %d. Found %d\n", contentSize, numBytes);
			break;
		}
		if((unsigned) atoi(buffer) != read % threadAmount){
			printf("Checker failed to read test file %s on iteration %d\n", testFileName, read);
			printf("Expected value %d. Found %s\n", read % threadAmount, buffer);
			break;
		}
	}

	delete [] buffer;
	delete openFile;
	return read == count * threadAmount;
}

// Forks a given amount of writer threads, that fill a given file collaboratively.
void SpawnWriters(void *spawnerArgs_){
	WriterSpawnerArg *spawnerArgs = (WriterSpawnerArg *) spawnerArgs_;
	unsigned fileNum = spawnerArgs -> fileNum; 
	unsigned writeSize = spawnerArgs -> writeSize;
	unsigned repCount = spawnerArgs -> repCount;
	unsigned threadAmount = spawnerArgs -> threadAmount;
	Semaphore *totalCheck = spawnerArgs -> totalCheck;
	
	char *testFileName = new char [64];
	snprintf(testFileName, 64, "%s%d", "ManyWriters ", fileNum);
	
	if(not fileSystem -> Create(testFileName, repCount * writeSize * threadAmount)){
		printf("Failed to create test file %s\n", testFileName);
		return;
	}
	
	WriterArg *args = new WriterArg[threadAmount];
	Semaphore* finishCheck = new Semaphore("TestMultipleWriters", 0);
	char *threadName = new char [64];
	for(unsigned threadNum = 0; threadNum < threadAmount; threadNum ++){		
		args[threadNum].fileName = testFileName;
		args[threadNum].writeSize = writeSize;
		args[threadNum].count = repCount;
		args[threadNum].threadAmount = threadAmount;
		args[threadNum].threadNum = threadNum;
		args[threadNum].finishCheck = finishCheck;
		
		snprintf(threadName, 64, "%s%d%s%d", "File ", fileNum, " Number ", threadNum);
		Thread *newThread = new Thread(threadName);
		newThread->Fork(WriterThread, (void*) (args + threadNum));
	}

	for(unsigned i = 0; i < threadAmount; i++)
		finishCheck -> P();

	if(CheckWriters(testFileName, writeSize, repCount, threadAmount)){
		if (not fileSystem->Remove(testFileName))
			printf("Test finished but failed to remove test file %s\n", testFileName);
	}
	else
		printf("!!!! Writers failed to write correctly on files %s.\n", testFileName);

    
    delete [] testFileName;
    delete [] args;
    delete finishCheck;
    delete [] threadName;
    
    
    totalCheck -> V();
}

// Forks multiple Writer Spawners, with each of them creating a file and forking writers to access it concurrently.
void TestWritersManyFiles(){
	unsigned fileAmount = 1;
	unsigned writeSize = 5;
	unsigned repCount = 100;
	unsigned threadAmount = 3;

	WriterSpawnerArg *args = new WriterSpawnerArg[fileAmount];
	char *spawnerName = new char[64];
	Semaphore *totalCheck = new Semaphore("TestWritersManyFiles", 0);
	for(unsigned fileNum = 0; fileNum < fileAmount; fileNum++){
		args[fileNum].fileNum = fileNum;
		args[fileNum].writeSize = writeSize;
		args[fileNum].repCount = repCount;
		args[fileNum].threadAmount = threadAmount;
		args[fileNum].totalCheck = totalCheck;
		
		snprintf(spawnerName, 64, "%s%d", "Spawner ", fileNum);
		Thread *newThread = new Thread(spawnerName);
		newThread->Fork(SpawnWriters, (void*) (args + fileNum));		
	}
	
	for(unsigned i = 0; i < fileAmount; i++)
		totalCheck -> P();
	
	delete [] args;
	delete [] spawnerName;
	delete totalCheck;	
	
	printf("--- TestWritersManyFiles successful!\n\n\n");
}



// Reads a section of the given test file byte by byte. If at any moment it reads a part that
// has not been yet overwritten by a writer, it waits on a condition variable until signalled.
void RWReaderThread(void* threadArgs_){
	RWReaderArg *threadArgs = (RWReaderArg *) threadArgs_;

	char* fileName = threadArgs -> fileName;
	unsigned start = threadArgs -> start;
	unsigned end = threadArgs -> end;
  char fillContent = threadArgs -> fillContent;
	Semaphore *finishCheck = threadArgs -> finishCheck;
	Lock *queueLock = threadArgs -> queueLock;
	Condition *queueCond = threadArgs -> queueCond;

	OpenFile *openFile = fileSystem->Open(fileName);
    if (openFile == nullptr) {
        printf("Reader %s was unable to open test file %s\n", currentThread -> GetName(), fileName);
        return;
    }

	char buffer;
	unsigned read;
	for(read = start; read < end; read++){
		unsigned numBytes = openFile -> ReadAt(&buffer, 1, read);
		while(numBytes == 1 and buffer == fillContent){
			queueLock -> Acquire();
			queueCond -> Wait();
			queueLock -> Release();
			
			numBytes = openFile -> ReadAt(&buffer, 1, read);			
		}
		if(numBytes < 1){
			printf("Reader %s failed to read test file %s on iteration %d\n", currentThread -> GetName(), fileName, read - start);
			break;
		}
	}

	delete openFile;

	if(read == end){
		//~ printf("Reader %s finished reading successfully!\n", currentThread -> GetName());
		finishCheck -> V();
	}
}

// Writes its thread ID in specific points in a given file, similarly to Writer Thread.
// After each successful write, it broadcasts readers so that they try and start reading again.
void RWWriterThread(void* threadArgs_){
	RWWriterArg *threadArgs = (RWWriterArg *) threadArgs_;

	char* fileName = threadArgs -> fileName;
	unsigned writeSize = threadArgs -> writeSize;
	unsigned count = threadArgs -> count;
	unsigned threadAmount = threadArgs -> threadAmount;
	unsigned threadNum = threadArgs -> threadNum;
	Semaphore *finishCheck = threadArgs -> finishCheck;
	Lock *queueLock = threadArgs -> queueLock;
	Condition *queueCond = threadArgs -> queueCond;

	OpenFile *openFile = fileSystem->Open(fileName);
    if (openFile == nullptr) {
        printf("Writer %s was unable to open test file %s\n", currentThread -> GetName(), fileName);
        return;
    }


	char* buffer = new char[writeSize + 1];
	snprintf(buffer, writeSize, "%d", threadNum);
	unsigned idLength = strlen(buffer);
	unsigned lenDiff = writeSize - idLength;
	
	for(unsigned i = 0; i < idLength; i++)
		buffer[i + lenDiff] = buffer[i];
		
	for(unsigned i = 0; i < lenDiff; i++)
		buffer[i] = '0';
		
	buffer[writeSize] = '\0';


	unsigned write;
	for(write = 0; write < count; write++){
		unsigned offset = writeSize * threadNum + write * writeSize * threadAmount;
		unsigned numBytes = openFile -> WriteAt(buffer, writeSize, offset);
        if (numBytes < writeSize) {
            printf("Writer %s failed to write test file %s on iteration %d\n", currentThread -> GetName(), fileName, write);
            break;
        }
        queueLock -> Acquire();
        queueCond -> Broadcast();
        queueLock -> Release();
	}

	delete [] buffer;
	delete openFile;

	if(write == count){
		//~ printf("Writer %s finished writing successfully!\n", currentThread -> GetName());
		finishCheck -> V();
	}
}

// Creates a file and forks both readers and writers to access it concurrently.
// Initially, the file consists of only hypens that are eventually overwritten by writers.
// If a reader find a hyphen, it waits on a condition variable until a writer accesses the file.
void SpawnReadersWriters(void *spawnerArgs_){
	RWSpawnerArg *spawnerArgs = (RWSpawnerArg *) spawnerArgs_;
	unsigned fileNum = spawnerArgs -> fileNum;
	unsigned writeSize = spawnerArgs -> writeSize;
	unsigned repetitionCount = spawnerArgs -> repCount;
	unsigned writerAmount = spawnerArgs -> writerAmount;
	unsigned readerAmount = spawnerArgs -> readerAmount;
	char fillContent = spawnerArgs -> fillContent;
	Semaphore *totalCheck = spawnerArgs -> totalCheck;
	
	char *testFileName = new char[64];
	snprintf(testFileName, 64, "%s%d", "ReadersWriters ", fileNum);
	
	unsigned fileSize = repetitionCount * writeSize * writerAmount;

	if(not WriteTestFile(testFileName, &fillContent, 1, fileSize)){
		printf("Failed to create test file %s\n", testFileName);
		return;
	}

	Semaphore* finishCheck = new Semaphore("TestReadersWriters", 0);
	Lock *queueLock = new Lock("ReadersQueueLock");
	Condition *queueCond = new Condition("ReadersQueueCondition", queueLock);

	char *threadName = new char [64];
	RWWriterArg* writerArgs = new RWWriterArg[writerAmount];
		
	for(unsigned threadNum = 0; threadNum < writerAmount; threadNum++){
		writerArgs[threadNum].fileName = testFileName;
		writerArgs[threadNum].writeSize = writeSize;
		writerArgs[threadNum].count = repetitionCount;
		writerArgs[threadNum].threadAmount = writerAmount;
		writerArgs[threadNum].threadNum = threadNum;
		writerArgs[threadNum].finishCheck = finishCheck;
		writerArgs[threadNum].queueLock = queueLock;
		writerArgs[threadNum].queueCond = queueCond;
		snprintf(threadName, 64, "%s%d%s%d", "File ", fileNum, "Number ", threadNum);
		Thread *newThread = new Thread(threadName);
		newThread->Fork(RWWriterThread, (void*) (writerArgs + threadNum));
	}
	
	
	RWReaderArg *readerArgs = new RWReaderArg[readerAmount];
	unsigned readSize = DivRoundUp(fileSize, readerAmount);
	for(unsigned threadNum = 0; threadNum < readerAmount; threadNum++){
		readerArgs[threadNum].fileName = testFileName;
		readerArgs[threadNum].start = readSize * threadNum;
		readerArgs[threadNum].end = minn((readSize + 1) * threadNum, fileSize);
		readerArgs[threadNum].fillContent = fillContent;
		readerArgs[threadNum].finishCheck = finishCheck;
		readerArgs[threadNum].queueLock = queueLock;
		readerArgs[threadNum].queueCond = queueCond;
		snprintf(threadName, 64, "%s%d%s%d", "File ", fileNum, "Number ", threadNum);
		Thread *newThread = new Thread(threadName);
		newThread->Fork(RWReaderThread, (void*) (readerArgs + threadNum));
	}
	
	for(unsigned i = 0; i < readerAmount + writerAmount; i++)
		finishCheck -> P();

	bool success = false;
	if(CheckWriters(testFileName, writeSize, repetitionCount, writerAmount)){
		if (not fileSystem->Remove(testFileName))
			printf("Test finished but failed to remove test file %s\n", testFileName);
			
		success = true;
	}
	else
		printf("!!!! TestReadersWriters unsuccessful: Writers failed to write correctly.\n\n\n");


    delete [] threadName;
    delete [] testFileName;
    delete queueCond;
    delete queueLock;
    delete finishCheck;
    delete [] writerArgs;
    delete [] readerArgs;
    
    if(success)
		totalCheck -> V();
}

// Forks multiple ReaderWriter Spawners, with each of them creating a file and forking both readers and writers to access it concurrently.
void TestReadersWritersManyFiles(){
	unsigned fileAmount = 1;
	unsigned repCount = 100;
	unsigned writeSize = 5;
	unsigned readerAmount = 10;
	unsigned writerAmount = 10;
	char fillContent = '-';

	RWSpawnerArg *args = new RWSpawnerArg[fileAmount];
	char *spawnerName = new char[64];
	Semaphore *totalCheck = new Semaphore("TestReadersManyFiles", 0);
	for(unsigned fileNum = 0; fileNum < fileAmount; fileNum++){
		args[fileNum].fileNum = fileNum;
		args[fileNum].writeSize = writeSize;
		args[fileNum].repCount = repCount;
		args[fileNum].writerAmount = writerAmount;
		args[fileNum].readerAmount = readerAmount;
		args[fileNum].fillContent = fillContent;
		args[fileNum].totalCheck = totalCheck;
		
		snprintf(spawnerName, 64, "%s%d", "Spawner ", fileNum);
		Thread *newThread = new Thread(spawnerName);
		newThread->Fork(SpawnReadersWriters, (void*) (args + fileNum));		
	}
	
	for(unsigned i = 0; i < fileAmount; i++)
		totalCheck -> P();
	
	delete [] args;
	delete [] spawnerName;
	delete totalCheck;
	
	printf("--- TestReadersWritersManyFiles successful!\n\n\n");	
}


// Checks that it is possible to remove a file that exists in the system but is not currently open.
void TestRemoveClosedFile(){
	char testName[] = "TestFile";
	if(not fileSystem -> Create(testName, 0)){
		printf("!!!! TestRemoveClosedFile failed. Unable to create test file.\n");
		return;
	}
	
	OpenFile *openFile = fileSystem -> Open(testName);
	if(openFile == nullptr){
		printf("!!!! TestRemoveClosedFile failed. Unable to open test file.\n");
		return;
	}
	delete openFile;
	
	if(not fileSystem -> Remove(testName)){
		printf("!!!! TestRemoveClosedFile failed. Unable to remove test file.\n");
		return;
	}
	
	openFile = fileSystem -> Open(testName);
	if(openFile != nullptr){
		printf("!!!! TestRemoveClosedFile failed. Removed file was still openable.\n");
		delete openFile;
		return;
	}
	
	printf("--- TestRemoveClosedFile successful!\n\n\n");
}



// Checks that it is possible to remove a file that is currently open, and that it is not openable anymore after that.
void TestRemoveOpenFile(){
	char testName[] = "TestFile";
	if(not fileSystem -> Create(testName, 0)){
		printf("!!!! TestRemoveOpenFile failed. Unable to create test file.\n");
		return;
	}
	
	OpenFile *openFile = fileSystem -> Open(testName);
	if(openFile == nullptr){
		printf("!!!! TestRemoveOpenFile failed. Unable to open test file.\n");
		return;
	}
	
	if(not fileSystem -> Remove(testName)){
		printf("!!!! TestRemoveOpenFile failed. Unable to remove open test file.\n");
		return;
	}
	
	OpenFile *openFile2 = fileSystem -> Open(testName);
	if(openFile2 != nullptr){
		printf("!!!! TestRemoveOpenFile failed. File pending to be removed was still openable.\n");
		delete openFile;
		delete openFile2;
		return;
	}
	
	delete openFile;
	
	openFile2 = fileSystem -> Open(testName);
	if(openFile2 != nullptr){
		printf("!!!! TestRemoveOpenFile failed. Removed file was still openable.\n");
		delete openFile2;
		return;
	}
	
	printf("--- TestRemoveOpenFile successful!\n\n\n");
}



// Checks that, if a closed file is removed multiple times, only the first one actually removes it and the rest do not find the file.
void TestMultipleRemovalsWhileClosed(){
	char testName[] = "TestFile";
	if(not fileSystem -> Create(testName, 0)){
		printf("!!!! TestMultipleRemovalsWhileClosed failed. Unable to create test file.\n");
		return;
	}
	
	OpenFile *openFile = fileSystem -> Open(testName);
	if(openFile == nullptr){
		printf("!!!! TestMultipleRemovalsWhileClosed failed. Unable to open test file.\n");
		return;
	}
	
	delete openFile;
	
	if(not fileSystem -> Remove(testName)){
		printf("!!!! TestMultipleRemovalsWhileClosed failed. Unable to remove test file.\n");
		return;
	}
	
	for(int i = 0; i < 5; i++){
		if(fileSystem -> Remove(testName)){
			printf("!!!! TestMultipleRemovalsWhileClosed failed. Able to removed already removed test file on iteration %d.\n", i);
			return;
		}
	}
		
	OpenFile *openFile2 = fileSystem -> Open(testName);
	if(openFile2 != nullptr){
		printf("!!!! TestMultipleRemovalsWhileClosed failed. Removed file was still openable.\n");
		delete openFile2;
		return;
	}
	
	printf("--- TestMultipleRemovalsWhileClosed successful!\n\n\n");	
}



// Checks that a file can be removed any number of times while open.
void TestMultipleRemovalsWhileOpen(){
	char testName[] = "TestFile";
	if(not fileSystem -> Create(testName, 0)){
		printf("!!!! TestMultipleRemovalsWhileOpen failed. Unable to create test file.\n");
		return;
	}
	
	OpenFile *openFile = fileSystem -> Open(testName);
	if(openFile == nullptr){
		printf("!!!! TestMultipleRemovalsWhileOpen failed. Unable to open test file.\n");
		return;
	}
	
	for(int i = 0; i < 5; i++){
		if(not fileSystem -> Remove(testName)){
			printf("!!!! TestMultipleRemovalsWhileOpen failed. Unable to remove open test file on iteration %d.\n", i);
			return;
		}
	}
		
	delete openFile;
	
	OpenFile *openFile2 = fileSystem -> Open(testName);
	if(openFile2 != nullptr){
		printf("!!!! TestMultipleRemovalsWhileOpen failed. Removed file was still openable.\n");
		delete openFile2;
		return;
	}
	
	printf("--- TestMultipleRemovalsWhileOpen successful!\n\n\n");	
}



// Checks that a file can be properly accessed even though it is pending to be removed.
void TestEditWhilePendingRemoval(){
	char testName[] = "TestFile";
	if(not fileSystem -> Create(testName, 0)){
		printf("!!!! TestEditWhilePendingRemoval failed. Unable to create test file.\n");
		return;
	}
	
	OpenFile *openFile = fileSystem -> Open(testName);
	if(openFile == nullptr){
		printf("!!!! TestEditWhilePendingRemoval failed. Unable to open test file.\n");
		return;
	}
	
	char contents[] = "1234567890";
	unsigned size = sizeof contents - 1;
	unsigned i;
	for(i = 0; i < 100; i++){
		unsigned numBytes = openFile -> Write(contents, size);
		if(numBytes < size)
			break;
	}
	
	if(i < 100){
		printf("!!!! TestEditWhilePendingRemoval failed. Unable to write prior to removal.\n");
		delete openFile;
		fileSystem -> Remove(testName);
		return;
	}
	
	if(not fileSystem -> Remove(testName)){
		printf("!!!! TestEditWhilePendingRemoval failed. Unable to remove open test file.\n");
		return;
	}
	
	
	for(i = 0; i < 100; i++){
		unsigned numBytes = openFile -> Write(contents, size);
		if(numBytes < size)
			break;
	}
	
	if(i < 100){
		printf("!!!! TestEditWhilePendingRemoval failed. Unable to write after removal.\n");
		delete openFile;
		return;
	}
	
	openFile -> Seek(0);
	char buffer[size];
	for(i = 0; i < 200; i++){
		unsigned numBytes = openFile -> Read(buffer, size);
		if(numBytes < size or strncmp(buffer, contents, size))
			break;
	}
	
	if(i < 200){
		printf("!!!! TestEditWhilePendingRemoval failed. Unable to read after removal or wrong results from reading.\n");
		delete openFile;
		return;
	}
	
	delete openFile;
	
	printf("--- TestEditWhilePendingRemoval successful!\n\n\n");	
}




