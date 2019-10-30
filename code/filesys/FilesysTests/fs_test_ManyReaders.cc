struct ReaderArg{
	char *fileName, *contents;
	unsigned contentSize, count;
	Semaphore *finishCheck;
};

struct TesterArg{
	char *testContents;
	unsigned fileNum, testContentSize, repCount, threadAmount;
	Semaphore *totalCheck;
};

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

void ReaderThread(void* threadArgs_){
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
            printf("Reader %s failed to read test file %s on interation %d\n", currentThread -> GetName(), fileName, read);
            break;
        }
	}

	delete [] buffer;
	delete openFile;

	if(read == count)
		printf("Reader %s finished reading successfully!\n", currentThread -> GetName());

	finishCheck -> V();
}

void TestManyReaders(void* testerArgs_){
	TesterArg *testerArgs = (TesterArg *) testerArgs_;
	char *testContents = testerArgs -> testContents;
	unsigned fileNum = testerArgs -> fileNum;
	unsigned testContentSize = testerArgs -> testContentSize;
	unsigned repCount = testerArgs -> repCount;
	unsigned threadAmount = testerArgs -> threadAmount;
	Semaphore *totalCheck = testerArgs -> totalCheck;
	
	char *testFileName = new char[64];
	snprintf(testFileName, 64, "%s%d", "MultipleReaders ", fileNum);

	if(not WriteTestFile(testFileName, testContents, testContentSize, repCount)){
		printf("Failed to create test file %s\n", testFileName);
		return;
	}
	
	Semaphore* finishCheck = new Semaphore("TestMultipleReaders", 0);
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

void TestReadersManyFiles(unsigned fileAmount){
	char testContents[] = "1234567890";
	unsigned testContentSize = sizeof testContents - 1;
	unsigned repCount = 100;
	unsigned threadAmount = 3;

	TesterArg *args = new TesterArg[fileAmount];
	char *testerName = new char[64];
	Semaphore *totalCheck = new Semaphore("TestReadersManyFiles", 0);
	for(unsigned fileNum = 0; fileNum < fileAmount; fileNum++){
		args[fileNum].testContents = testContents;
		args[fileNum].fileNum = fileNum;
		args[fileNum].testContentSize = testContentSize;
		args[fileNum].repCount = repCount;
		args[fileNum].threadAmount = threadAmount;
		args[fileNum].totalCheck = totalCheck;
		
		snprintf(testerName, 64, "%s%d", "Tester ", fileNum);
		Thread *newThread = new Thread(testerName);
		newThread->Fork(TestManyReaders, (void*) (args + fileNum));		
	}
	
	
	for(unsigned i = 0; i < fileAmount; i++)
		totalCheck -> P();
	
	printf("-- TestReadersManyFiles successful!\n\n\n");
	
	delete [] args;
	delete totalCheck;
	delete [] testerName;
}
