// Usage: define PORT_BLOCK_TEST in Makefile and run ThreadTest
// This test case must be called without random yields.
Port* testPort = new Port("Test Port");
Semaphore* finishCheck = new Semaphore("finishCheckSemaphore", 0);
bool senderFlag = false;
bool receiverFlag = false;

TestPortBlockStruct* testStructSender = new TestPortBlockStruct;
testStructSender -> port = testPort;
testStructSender -> finishCheck = finishCheck;
testStructSender -> testFlag = &senderFlag;

Thread* sender = new Thread("Sender");
sender -> Fork(PortTestSender, (void *) testStructSender);
currentThread -> Yield();

ASSERT(!senderFlag);

TestPortBlockStruct* testStructReceiver = new TestPortBlockStruct;
testStructReceiver -> port = testPort;
testStructReceiver -> finishCheck = finishCheck;
testStructReceiver -> testFlag = &receiverFlag;

Thread* receiver = new Thread("Receiver");
receiver -> Fork(PortTestReceiver, (void *) testStructReceiver);
currentThread -> Yield();

finishCheck -> P();
finishCheck -> P();

printf("!!! Blocking Send Test success.\n");

senderFlag = false;
receiverFlag = false;

Thread* receiver2 = new Thread("Receiver2");
receiver2 -> Fork(PortTestReceiver, (void *) testStructReceiver);
currentThread -> Yield();

ASSERT(!receiverFlag);

Thread* sender2 = new Thread("Sender2");
sender2 -> Fork(PortTestSender, (void *) testStructSender);
currentThread -> Yield();

finishCheck -> P();
finishCheck -> P();

printf("!!! Blocking Receive Test success.\n");

delete testPort;
delete finishCheck;
delete testStructSender;
delete testStructReceiver;
