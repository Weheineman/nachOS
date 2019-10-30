// Test routines to check how nachOS handles
// access to the file system by many threads simultaneously.

#include "filesys/file_system.hh"
#include "lib/utility.hh"
#include "machine/disk.hh"
#include "machine/statistics.hh"
#include "threads/thread.hh"
#include "threads/system.hh"

struct MultipleReaderArg{
	char *fileName, *contents;
	unsigned contentSize, count;
	Semaphore *finishCheck;
};

struct MultipleReaderTesterArg{
	char *testContents;
	unsigned fileNum, testContentSize, repCount, threadAmount;
	Semaphore *totalCheck;
};

struct MultipleWriterArg{
	char *fileName;
	unsigned writeSize, count, threadAmount;
	Semaphore *finishCheck;
};

struct RWReaderArg{
	char *fileName;
	unsigned start, end;
	Semaphore *finishCheck;
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

void MultipleReaderThread(void* threadArgs_){
	MultipleReaderArg *threadArgs = (MultipleReaderArg *) threadArgs_;

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

void TestMultipleReaders(void* testerArgs_){
	MultipleReaderTesterArg *testerArgs = (MultipleReaderTesterArg *) testerArgs_;
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
	MultipleReaderArg* threadArgs = new MultipleReaderArg;
	threadArgs -> fileName = testFileName;
	threadArgs -> contents = testContents;
	threadArgs -> contentSize = testContentSize;
	threadArgs -> count = repCount;
	threadArgs -> finishCheck = finishCheck;

	char *threadName = new char [64];
	for(unsigned threadNum = 0; threadNum < threadAmount; threadNum ++){
		snprintf(threadName, 64, "%s%d%s%d", "File ", fileNum, " Number ", threadNum);
		Thread *newThread = new Thread(threadName);
		newThread->Fork(MultipleReaderThread, (void*) threadArgs);
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

	MultipleReaderTesterArg *args = new MultipleReaderTesterArg[fileAmount];
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
		newThread->Fork(TestMultipleReaders, (void*) (args + fileNum));		
	}
	
	
	for(unsigned i = 0; i < fileAmount; i++)
		totalCheck -> P();
	
	printf("-- TestReadersManyFiles successful!\n\n\n");
	
	delete [] args;
	delete totalCheck;
	delete [] testerName;
}

void MultipleWriterThread(void* threadArgs_){
	MultipleWriterArg *threadArgs = (MultipleWriterArg *) threadArgs_;

	char* fileName = threadArgs -> fileName;
	unsigned writeSize = threadArgs -> writeSize;
	unsigned count = threadArgs -> count;
	unsigned threadAmount = threadArgs -> threadAmount;
	Semaphore *finishCheck = threadArgs -> finishCheck;


	OpenFile *openFile = fileSystem->Open(fileName);
    if (openFile == nullptr) {
        printf("Writer %s was unable to open test file %s\n", currentThread -> GetName(), fileName);
        return;
    }

	char* threadName = currentThread -> GetName();
	int threadNum = atoi(threadName);

	char* buffer = new char[writeSize + 1];
	unsigned i;
	for(i = 0; i < writeSize - strlen(threadName); i++)
		buffer[i] = '0';
	strncpy(buffer + i, threadName, writeSize - i);
	buffer[writeSize] = '\0';

	unsigned write;
	for(write = 0; write < count; write++){
		unsigned offset = writeSize * threadNum + write * writeSize * threadAmount;
		// printf("Thread %s writing in offset %d on iteration %d\n", threadName, offset, write);
		unsigned numBytes = openFile -> WriteAt(buffer, writeSize, offset);
        if (numBytes < writeSize) {
            printf("Writer %s failed to write test file %s on interation %d\n", threadName, fileName, write);
            break;
        }
	}

	delete [] buffer;
	delete openFile;

	if(write == count)
		printf("Writer %s finished writing successfully!\n", threadName);

	finishCheck -> V();
}

bool CheckMultipleWriters(char *testFileName, unsigned contentSize, unsigned count, unsigned threadAmount){
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
			printf("Checker failed to read test file %s on interation %d\n", testFileName, read);
			printf("Expected read size %d. Found %d\n", contentSize, numBytes);
			break;
		}
		if((unsigned) atoi(buffer) != read % threadAmount){
			printf("Checker failed to read test file %s on interation %d\n", testFileName, read);
			printf("Expected value %d. Found %s\n", read % threadAmount, buffer);
			break;
		}
	}

	delete [] buffer;
	delete openFile;
	return read == count * threadAmount;
}

void TestMultipleWriters(){
	char testFileName[] = "MultipleWriters";
	unsigned repetitionCount = 100;
	unsigned writeSize = 4;

	unsigned threadAmount = 10;

	if(not fileSystem -> Create(testFileName, repetitionCount * writeSize * threadAmount)){
		printf("Failed to create test file %s\n", testFileName);
		return;
	}

	Semaphore* finishCheck = new Semaphore("TestMultipleWriters", 0);
	MultipleWriterArg* threadArgs = new MultipleWriterArg;
	threadArgs -> fileName = testFileName;
	threadArgs -> writeSize = writeSize;
	threadArgs -> count = repetitionCount;
	threadArgs -> threadAmount = threadAmount;
	threadArgs -> finishCheck = finishCheck;


	char *threadName = new char [64];
	for(unsigned threadNum = 0; threadNum < threadAmount; threadNum ++){
		snprintf(threadName, 64, "%d", threadNum);
		Thread *newThread = new Thread(threadName);
		newThread->Fork(MultipleWriterThread, (void*) threadArgs);
	}

	for(unsigned i = 0; i < threadAmount; i++)
		finishCheck -> P();

  if(CheckMultipleWriters(testFileName, writeSize, repetitionCount, threadAmount)){
		if (not fileSystem->Remove(testFileName))
					printf("Test finished but failed to remove test file %s\n", testFileName);

		printf("-- TestMultipleWriters successful!\n\n\n");
	}
	else
  	printf("!!!! TestMultipleWriters unsuccessful: Writers failed to write correctly.\n\n\n");

    delete threadName;
    delete threadArgs;
}

void RWReaderThread(void* threadArgs_){
	RWReaderArg *threadArgs = (RWReaderArg *) threadArgs_;

	char* fileName = threadArgs -> fileName;
	unsigned start = threadArgs -> start;
	unsigned end = threadArgs -> end;
	Semaphore *finishCheck = threadArgs -> finishCheck;

	OpenFile *openFile = fileSystem->Open(fileName);
    if (openFile == nullptr) {
        printf("Reader %s was unable to open test file %s\n", currentThread -> GetName(), fileName);
        return;
    }

	char buffer;
	unsigned read;
	for(read = start; read < end; read++){
		unsigned numBytes = openFile -> ReadAt(&buffer, 1, read);
		while(numBytes == 1 and buffer == '-'){
			currentThread -> Yield();
			numBytes = openFile -> ReadAt(&buffer, 1, read);			
		}
		if(numBytes < 1){
			printf("Reader %s failed to read test file %s on interation %d\n", currentThread -> GetName(), fileName, read - start);
			break;
		}
	}

	delete openFile;

	if(read == end)
		printf("Reader %s finished reading successfully!\n", currentThread -> GetName());

	finishCheck -> V();
}

void TestReadersWriters(){
	char testFileName[] = "ReadersWriters";
	char contents[] = "-";
	unsigned repetitionCount = 100;
	unsigned writeSize = 5;

	unsigned readerAmount = 10;
	unsigned writerAmount = 10;

	unsigned fileSize = repetitionCount * writeSize * writerAmount;

	if(not WriteTestFile(testFileName, contents, 1, fileSize)){
		printf("Failed to create test file %s\n", testFileName);
		return;
	}

	Semaphore* finishCheck = new Semaphore("TestReadersWriters", 0);

	char *threadName = new char [64];
	MultipleWriterArg* writerArgs = new MultipleWriterArg;
	writerArgs -> fileName = testFileName;
	writerArgs -> writeSize = writeSize;
	writerArgs -> count = repetitionCount;
	writerArgs -> threadAmount = writerAmount;
	writerArgs -> finishCheck = finishCheck;
	
	for(unsigned threadNum = 0; threadNum < writerAmount; threadNum++){
		snprintf(threadName, 64, "%d", threadNum);
		Thread *newThread = new Thread(threadName);
		newThread->Fork(MultipleWriterThread, (void*) writerArgs);
	}
	
	
	RWReaderArg *readerArgs = new RWReaderArg[readerAmount];
	unsigned readSize = DivRoundUp(fileSize, readerAmount);
	for(unsigned threadNum = 0; threadNum < readerAmount; threadNum++){
		readerArgs[threadNum].fileName = testFileName;
		readerArgs[threadNum].start = readSize * threadNum;
		readerArgs[threadNum].end = minn((readSize + 1) * threadNum, fileSize);
		readerArgs[threadNum].finishCheck = finishCheck;
		
		snprintf(threadName, 64, "%d", threadNum);
		Thread *newThread = new Thread(threadName);
		newThread->Fork(RWReaderThread, (void*) (readerArgs + threadNum));
	}
	
	for(unsigned i = 0; i < readerAmount + writerAmount; i++)
		finishCheck -> P();

	if(CheckMultipleWriters(testFileName, writeSize, repetitionCount, writerAmount)){
		if (not fileSystem->Remove(testFileName))
			printf("Test finished but failed to remove test file %s\n", testFileName);

		printf("-- TestReadersWriters successful!\n\n\n");
	}
	else
		printf("!!!! TestReadersWriters unsuccessful: Writers failed to write correctly.\n\n\n");

    delete threadName;
    delete writerArgs;
    delete [] readerArgs;
}

void TestSimpleMultipleFiles(){
	char file1[] = "Test 1";
	char file2[] = "Test 2";
	
	unsigned count = 100;
	char contents[] = "42069";
	unsigned size = sizeof contents - 1 * count;
	
	if(not fileSystem -> Create(file1, size)){
		printf("Cannot create test file %s\n", file1);
		return;
	}
	OpenFile *openFile1 = fileSystem->Open(file1);
    if(openFile1 == nullptr) {
        printf("Unable to open test file %s\n", file1);
        return;
    }
    
	
	if(not fileSystem -> Create(file2, size)){
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

	delete openFile1;
	delete openFile2;
	
	if(i == count)
		printf("-- TestSimpleMultipleFiles successful!\n\n\n");
	else
		printf("!!!! TestSimpleMultipleFiles unsuccessful: Writers failed to write correctly.\n\n\n");
}


void FileSysConcurrencyTests(){
	TestSimpleMultipleFiles();
	TestReadersManyFiles(2);
	//~ TestMultipleReaders();
	//~ TestMultipleWriters();
	//~ TestReadersWriters();
}
