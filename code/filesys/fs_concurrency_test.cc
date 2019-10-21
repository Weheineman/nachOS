// Test routines to check how nachOS handles
// access to the file system by many threads simultaneously.

#include "file_system.hh"
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

struct MultipleWriterArg{
	char *fileName;
	unsigned writeSize, count, threadAmount;
	Semaphore *finishCheck;
};

bool WriteTestFile(char name[], char contents[], unsigned size, unsigned count){
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
  for (i = 0; i < count; i += 1) {
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
        printf("Thread %s was unable to open test file %s\n", currentThread -> GetName(), fileName);
        return;
    }

	char* buffer = new char[contentSize];
	unsigned read;
	for(read = 0; read < count; read++){
		unsigned numBytes = openFile -> Read(buffer, contentSize);
        if (numBytes < contentSize || strncmp(buffer, contents, contentSize)) {
            printf("Thread %s failed to read test file %s on interation %d\n", currentThread -> GetName(), fileName, read);
            break;
        }
	}

	delete [] buffer;
	delete openFile;

	if(read == count)
		printf("Thread %s finished reading successfully!\n", currentThread -> GetName());

	finishCheck -> V();
}

void TestMultipleReaders(){
	char testFileName[] = "MultipleReaders";
	char testContents[] = "1234567890";
	unsigned testContentSize = sizeof testContents - 1;
	unsigned repetitionCount = 1000;

	unsigned threadAmount = 10;

	if(not WriteTestFile(testFileName, testContents, testContentSize, repetitionCount)){
		printf("Failed to create test file %s\n", testFileName);
		return;
	}

	Semaphore* finishCheck = new Semaphore("TestMultipleReaders", 0);
	MultipleReaderArg* threadArgs = new MultipleReaderArg;
	threadArgs -> fileName = testFileName;
	threadArgs -> contents = testContents;
	threadArgs -> contentSize = testContentSize;
	threadArgs -> count = repetitionCount;
	threadArgs -> finishCheck = finishCheck;


	char *threadName = new char [64];
	for(unsigned threadNum = 0; threadNum < threadAmount; threadNum ++){
		snprintf(threadName, 64, "%s%d", "Number ", threadNum);
		Thread *newThread = new Thread(threadName);
		newThread->Fork(MultipleReaderThread, (void*) threadArgs);
	}

	for(unsigned i = 0; i < threadAmount; i++)
		finishCheck -> P();

	if (not fileSystem->Remove(testFileName))
        printf("Test finished but failed to remove test file %s\n", testFileName);

    printf("-- TestMultipleReaders successful!\n\n\n");

    delete threadName;
    delete threadArgs;
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
        printf("Thread %s was unable to open test file %s\n", currentThread -> GetName(), fileName);
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
            printf("Thread %s failed to write test file %s on interation %d\n", threadName, fileName, write);
            break;
        }
	}

	delete [] buffer;
	delete openFile;

	if(write == count)
		printf("Thread %s finished writing successfully!\n", threadName);

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

void FileSysConcurrencyTests(){
	TestMultipleReaders();
	TestMultipleWriters();
	// TestReadersWriters();
}
