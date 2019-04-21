/// Simple test case for the threads assignment.
///
/// Create several threads, and have them context switch back and forth
/// between themselves by calling `Thread::Yield`, to illustrate the inner
/// workings of the thread system.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "system.hh"
#include "synch.hh"

struct TestLockStruct {
	int *testVariable;
	Lock *testLock;
	Semaphore *finishCheck;
};

struct TestCondStruct {
	unsigned int bufferSize;
	List<char*> *buffer;
	Condition *testConditionProd;
	Condition *testConditionCons;
	Lock *condLock;
	Semaphore *finishCheck;
	unsigned int amount;
};

/// Loop 10 times, yielding the CPU to another ready thread each iteration.
///
/// *dummy exists only to satisfy the function type necessary for Fork
void
SimpleThread(void *dummy)
{
    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.
    for (unsigned num = 0; num < 10; num++) {
        printf("*** Thread `%s` is running: iteration %u\n",
		       currentThread->GetName(), num);
        currentThread->Yield();
    }

    printf("!!! Thread `%s` has finished\n", currentThread->GetName());
}


// Same as SimpleThread but is called only if SEMAPHORE_TEST is defined.
// Uses the semaphore declared in ThreadTest.
void SemaphoreThread(void *testSemaphore_){
    // Reinterpret arg `pointerPair` as a Semaphore pointer.
    Semaphore *testSemaphore = (Semaphore*) testSemaphore_;

    testSemaphore->P();

    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.
    for (unsigned num = 0; num < 10; num++) {
        printf("*** Thread `%s` is running: iteration %u\n",
		       currentThread->GetName(), num);
        currentThread->Yield();
    }

    printf("!!! Thread `%s` has finished\n", currentThread->GetName());

    testSemaphore->V();
}

void LockThread(void *structPointer_){
    // Reinterpret arg `pointerPair` as a pair<*char, *Lock> pointer.
    TestLockStruct *structPointer = (TestLockStruct*) structPointer_;

    // Rename the elements of pointerPair for more clarity
    int *testVariable = structPointer -> testVariable;
    Lock *testLock = structPointer -> testLock;
	Semaphore *finishCheck = structPointer -> finishCheck;

    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.
    int currentValue;
	unsigned int iterationNumber = 100;
    for (unsigned num = 0; num < iterationNumber; num++) {
		testLock -> Acquire();
        currentValue = *testVariable;
		currentValue++;
        *testVariable = currentValue;
		currentThread->Yield();
		testLock -> Release();
    }
    printf("!!! Thread `%s` has finished\n", currentThread->GetName());

	// Increase the value of the finishCheck Semaphore
    finishCheck -> V();
}

void
CondTestProducer(void *structPointer_)
{
	TestCondStruct *structPointer = (TestCondStruct*) structPointer_;

	unsigned int bufferSize = structPointer -> bufferSize;
	List<char*> *buffer = structPointer -> buffer;
	Condition *testConditionProd = structPointer -> testConditionProd;
	Condition *testConditionCons = structPointer -> testConditionCons;
	Lock *condLock = structPointer -> condLock;
	unsigned int produceTotal = structPointer -> amount;

	unsigned int produceAmount = 0;

	while(produceAmount < produceTotal){
		condLock->Acquire();

		while(buffer->Length() == bufferSize){
			testConditionProd->Wait();
		}

		char *producerName = new char [64];
		strcpy(producerName, currentThread->GetName());
		buffer->Append(producerName);
		produceAmount++;
		printf("I'm producer %s and I'm producing memes for the %d th time. \n",
			   currentThread->GetName(), produceAmount);
		testConditionCons->Broadcast();

		condLock->Release();
	}

	printf("!!! Thread Producer `%s` has finished\n",currentThread->GetName());
}

void
CondTestConsumer(void *structPointer_)
{
	TestCondStruct *structPointer = (TestCondStruct*) structPointer_;

	List<char*> *buffer = structPointer -> buffer;
	Condition *testConditionProd = structPointer -> testConditionProd;
	Condition *testConditionCons = structPointer -> testConditionCons;
	Lock *condLock = structPointer -> condLock;
	Semaphore *finishCheck = structPointer -> finishCheck;
	unsigned int consumeTotal = structPointer -> amount;

	unsigned int consumeAmount = 0;
	char *producerName;

	while(consumeAmount < consumeTotal){
		condLock->Acquire();

		while(buffer->IsEmpty()){
			testConditionCons->Wait();
		}

		producerName = buffer->Pop();
		consumeAmount++;
		printf("I'm consumer %s and producer %s sent me memes. \n",
		       currentThread->GetName(), producerName);
		delete producerName;
		testConditionProd->Broadcast();

		condLock->Release();
	}

	printf("!!! Thread Consumer `%s` has finished\n", currentThread->GetName());
	finishCheck -> V();
}

/// Set up a ping-pong between several threads.
///
/// Do it by launching ten threads which call `SimpleThread`, and finally
/// calling `SimpleThread` ourselves.
void
ThreadTest()
{
	DEBUG('t', "Entering thread test\n");
    // Amount of threads to launch
    const int threadAmount = 3;

    #ifdef SEMAPHORE_TEST
    // Initial value of the semaphore
    const int semInit = 3;
    Semaphore *testSemaphore = new Semaphore("Ejercicio 15", semInit);

	#elif defined LOCK_TEST

	int testVariable = 0;
	Lock *testLock = new Lock("Test Lock");
	Semaphore *finishCheck = new Semaphore("finishCheckSemaphore", 0);

	TestLockStruct *testStruct = new TestLockStruct;
	testStruct -> testVariable = &testVariable;
	testStruct -> testLock = testLock;
	testStruct -> finishCheck = finishCheck;

	#elif defined COND_TEST

	unsigned int bufferSize = 5;
	List<char*> *buffer = new List<char*>;
	Lock *condLock = new Lock("Test Lock for Condition Variable");
	Condition *testConditionProd = new Condition("Condition for Producers", condLock);
	Condition *testConditionCons = new Condition("Condition for Consumers", condLock);
	Semaphore *finishCheck = new Semaphore("finishCheckSemaphore", 0);

	TestCondStruct *testStruct = new TestCondStruct;
	testStruct -> bufferSize = bufferSize;
	testStruct -> buffer = buffer;
	testStruct -> condLock = condLock;
	testStruct -> testConditionProd = testConditionProd;
	testStruct -> testConditionCons = testConditionCons;
	testStruct -> finishCheck = finishCheck;
	testStruct -> amount = 5;
	#endif

    // name will be used to generate the thread names
    char *name = new char [64];
    for(int threadNum = 1; threadNum <= threadAmount; threadNum++){
        snprintf(name, 64, "%s%d", "Number ", threadNum);
        Thread *newThread = new Thread(name);

        #ifdef SEMAPHORE_TEST

        // Launch semaphore test threads
        newThread->Fork(SemaphoreThread, (void*) testSemaphore);

        #elif defined LOCK_TEST

		// Launch lock test threads
        newThread->Fork(LockThread, (void*) testStruct);

        #elif defined COND_TEST

		// Launch condition variable test (consumer/producer) threads
        snprintf(name, 64, "%s%d", "Number' ", threadNum);
        Thread *newThread2 = new Thread(name);

		newThread->Fork(CondTestProducer, (void*) testStruct);
		newThread2->Fork(CondTestConsumer, (void*) testStruct);

        #else
        // Launch simple threads
		void *dummy = nullptr;
        newThread->Fork(SimpleThread, dummy);
        #endif
    }

	delete [] name;

    #ifdef LOCK_TEST
    for(int i = 0; i < threadAmount; i++)
        finishCheck->P();
    char lockTestMsg[64];
    snprintf(lockTestMsg, 64, "Lock test variable value: %d \n", testVariable);

	printf("%s\n", lockTestMsg);

	delete testLock;
	delete finishCheck;
    delete testStruct;

    #elif defined COND_TEST
    for(int i = 0; i < threadAmount; i++)
        finishCheck->P();

    delete buffer;
	printf("Deleted buffer\n");
    delete condLock;
	printf("Deleted condLock\n");
    delete testConditionProd;
	printf("Deleted testProd\n");
    delete testConditionCons;



	printf("Deleted testCons\n");
	printf("%s\n", finishCheck->GetName());

	// Uncomment the next line to get a segmentation fault :)
    // delete finishCheck;

	printf("Deleted finishCheck\n");
    delete testStruct;
	printf("Deleted testStruct\n");
    #endif

    DEBUG('t', "Exiting thread test\n");
}
