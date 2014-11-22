

// threadtest.cc 
//  Test cases for the threads assignment.

#include "copyright.h"
#include "system.h"
#include "synch.h"

// testnum is set in main.cc
int testnum = 1;

//----------------------------------------------------------------------
// SimpleThread
//  Loop 5 times, yielding the CPU to another ready thread 
//  each iteration.
//
//  "which" is simply a number identifying the thread, for debugging
//  purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
    printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest1
//  Set up a ping-pong between two threads, by forking a thread 
//  to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

//----------------------------------------------------------------------
// LockTest1
//----------------------------------------------------------------------

Lock *locktest1 = NULL;

void
LockThread1(int param)
{
    printf("L1:0\n");
    locktest1->Acquire();
    printf("L1:1\n");
    currentThread->Yield();
    printf("L1:2\n");
    locktest1->Release();
    printf("L1:3\n");
}

void
LockThread2(int param)
{
    printf("L2:0\n");
    locktest1->Acquire();
    printf("L2:1\n");
    currentThread->Yield();
    printf("L2:2\n");
    locktest1->Release();
    printf("L2:3\n");
}


void
LockTest1()
{
    DEBUG('t', "Entering LockTest1");

    locktest1 = new Lock("LockTest1");

    Thread *t = new Thread("one");
    t->Fork(LockThread1, 0);
    t = new Thread("two");
    t->Fork(LockThread2, 0);
}

//----------------------------------------------------------------------
// acquireSameLock
//----------------------------------------------------------------------
Lock *sameLock = NULL;

void
acquireLock1(int param){
    printf("Acquiring lock once\n");
    sameLock->Acquire();
    printf("Acquired lock once. Yielding to different thread\n");
    currentThread->Yield();
    printf("Releasing first lock\n");
    sameLock->Release();
    printf("Released the first lock\n");
}

void
acquireLock2(int param){
    printf("Attempting to aquire the same lock again\n");
    sameLock->Acquire();
    printf("Acquired lock twice. Yielding to a different thread\n");
    currentThread->Yield();
    printf("Attempting to release the second lock (same lock)\n");
    sameLock->Release();
    printf("Released the sameLock the second time around\n");
}

void
acquireSameLock(){
    DEBUG('t', "Entering acquireSameLock");

    sameLock = new Lock("sameLock");

    Thread *t = new Thread("one");
    t->Fork(acquireLock1, 0);
    t =new Thread("two");
    t->Fork(acquireLock2, 0);
}

//----------------------------------------------------------------------
// acquireSameLock2
// Checks to see that the same thread cannot acquire the same lock twice
//----------------------------------------------------------------------
void
acquireLock3(int param){
    printf("Attempting to acquire the lock for the first time\n");
    sameLock->Acquire();
    printf("Acquired the lock\n");
}

void
acquireLock4(int param){
    printf("Attempting to acquire the lock for the second time\n");
    sameLock->Acquire();
    printf("Acquired the lock the second time. This should not print\n");
}
void
acquireSameLock2(){
    DEBUG('t', "Entering acquireSameLock2");

    sameLock = new Lock("sameLock");

    Thread *t = new Thread("one");
    t->Fork(acquireLock4, 0);
    t = new Thread("two");
    t->Fork(acquireLock3, 0);
}


//----------------------------------------------------------------------
// releaseUnheldLock
// Releasing a Lock that isn't held
//----------------------------------------------------------------------
Lock *unheldLock = NULL;

void
releaseLock1(int param){
    printf("Attempting to release an unheld lock\n");
    unheldLock->Release();
    printf("Unheld lock has been released. This should not be printed.\n");
}

void
releaseUnheldLock(){
    DEBUG('t',"Entering releaseUnheldLock");

    unheldLock = new Lock("unheldLock");

    Thread *t = new Thread("one");
    t->Fork(releaseLock1,0);

}

//----------------------------------------------------------------------
// deleteUnheldLock
// Deleting a lock that is not held
//----------------------------------------------------------------------
void
deleteLock1(int param){
    printf("Attempting to delete an unheld lock\n");
    delete unheldLock;
    printf("Deleted the unheld lock\n");
}

void
deleteUnheldLock(){
    DEBUG('t',"Entering deleteUnheldLock");
    unheldLock = new Lock("unheldLock");

    Thread *t = new Thread("one");
    t->Fork(deleteLock1,0);

}

//----------------------------------------------------------------------
// deleteHeldLock
// Deleting a locking a lock that is held
//----------------------------------------------------------------------
Lock *heldLock = NULL;

void
deleteLock2(int param){
    printf("Acquiring the lock\n");
    heldLock->Acquire();
    printf("Lock has been held\n");
    printf("Attempting to delete the held lock\n");
    delete heldLock;
    printf("Deleted the held lock. This statement shoudl not print\n");
}
void
deleteHeldLock(){
    DEBUG('t',"Entering deleteHeldLock");
    heldLock = new Lock("heldLock");

    Thread *t = new Thread("one");
    t->Fork(deleteLock2,0);
}

//----------------------------------------------------------------------
// waitCondUnheldLock
// Waiting on a condition variable without holding a Lock
//----------------------------------------------------------------------
Condition *testCond = NULL;
void
waitUnheldLock1(int param){
    printf("Attempting to wait on a condition variable on unheld lock\n");
    testCond->Wait(unheldLock);
    printf("This should not print\n");
}

void
waitCondUnheldLock(){
    DEBUG('t',"Entering waitCondUnheldLock");

    unheldLock = new Lock("unheldLock");
    testCond = new Condition("testCond");

    Thread *t = new Thread("one");
    t->Fork(waitUnheldLock1,0);   
}


//----------------------------------------------------------------------
// signalThread
// Signalling a condition variable wakes only one thread
//----------------------------------------------------------------------
void
signalThread1(int param){
    printf("Thread 1 acquiring the lock\n");
    heldLock->Acquire();
    printf("Lock has been acquired\n");
    printf("Wait (sleep) thread 1\n");
    testCond->Wait(heldLock);
    printf("Thread 1 has woken up\n");
    heldLock->Release();

}

void
signalThread2(int param){
    printf("Thread 2 acquiring the lock\n");
    heldLock->Acquire();
    printf("Lock has been acquired\n");
    printf("Wait (sleep) thread 2\n");
    testCond->Wait(heldLock);
    printf("Thread 2 has woken up\n");
    heldLock->Release();
}

void
signalThread3(int param){
    printf("Thread 3 acquiring the lock\n");
    heldLock->Acquire();
    printf("Lock has been acquired\n");
    printf("Wake up one thread\n");
    testCond->Signal(heldLock);
    printf("Only thread 1 should have woken up\n");
    heldLock->Release();
}

void
signalThread(){
    DEBUG('t',"Entering signalThread");

    heldLock = new Lock("heldLock");
    testCond = new Condition("testCond");

    Thread *t = new Thread("one");
    t->setPriority(3);
    t->Fork(signalThread1,0);  
    t = new Thread("two");
    t->setPriority(2);
    t->Fork(signalThread2,0);
    t = new Thread("three");
    t->setPriority(1);
    t->Fork(signalThread3,0);
}

//----------------------------------------------------------------------
// broadcastThread
// Broadcasting a condition variable wakes all threads
//----------------------------------------------------------------------
void
broadcastThread1(int param){
    printf("Thread 1 acquiring the lock\n");
    heldLock->Acquire();
    printf("Lock has been acquired\n");
    printf("Wait (sleep) thread 1\n");
    testCond->Wait(heldLock);
    printf("Thread 1 has woken up\n");
    heldLock->Release();

}

void
broadcastThread2(int param){
    printf("Thread 2 acquiring the lock\n");
    heldLock->Acquire();
    printf("Lock has been acquired\n");
    printf("Wait (sleep) thread 2\n");
    testCond->Wait(heldLock);
    printf("Thread 2 has woken up\n");
    heldLock->Release();
}

void
broadcastThread3(int param){
    printf("Thread 3 acquiring the lock\n");
    heldLock->Acquire();
    printf("Lock has been acquired\n");
    printf("Wake up one thread\n");
    testCond->Broadcast(heldLock);
    printf("Both threads 1 and 2 should wake up\n");
    heldLock->Release();
}

void
broadcastThread(){
    DEBUG('t',"Entering broadcastThread");

    heldLock = new Lock("heldLock");
    testCond = new Condition("testCond");

    Thread *t = new Thread("one");
    t->setPriority(3);
    t->Fork(broadcastThread1,0);  
    t = new Thread("two");
    t->setPriority(2);
    t->Fork(broadcastThread2,0);
    t = new Thread("three");
    t->setPriority(1);
    t->Fork(broadcastThread3,0);
}

//----------------------------------------------------------------------
// noWaitSignal
// Signalling to a condition variable with no waiters is a no-op
//----------------------------------------------------------------------
void
noWaitSignal1(int param){
    printf("Acquiring the lock\n");
    heldLock->Acquire();
    printf("Lock has been acquired\n");

    printf("Attempt to signal a CV with no waiters\n");
    testCond->Signal(heldLock);

    printf("This statement should print afterit is stated there are no waiters\n");
}

void
noWaitSignal(){
    DEBUG('t',"Entering noWaitSignal");

    heldLock = new Lock("heldLock");
    testCond = new Condition("testCond");

    Thread *t = new Thread("one");
    t->Fork(noWaitSignal1,0);  
}


//----------------------------------------------------------------------
// noWaitBroadcast
// Calling broadcast to a condition variable with no waiters is a no-op
//----------------------------------------------------------------------
void
noWaitBroadcast1(int param){
    printf("Acquiring the lock\n");
    heldLock->Acquire();
    printf("Lock has been acquired\n");

    printf("Attempt to broadcast a CV with no waiters\n");
    testCond->Broadcast(heldLock);

    printf("This statement should print afterit is stated there are no waiters\n");
}

void
noWaitBroadcast(){
    DEBUG('t',"Entering noWaitBroadcast");

    heldLock = new Lock("heldLock");
    testCond = new Condition("testCond");

    Thread *t = new Thread("one");
    t->Fork(noWaitBroadcast1,0);  
}

//----------------------------------------------------------------------
// signalLockCall
// A thread calling Signal holds the Lock passed in to Signal
//----------------------------------------------------------------------
Lock *secondaryLock;
void
signalLockCall1(int param){
    printf("Thread 1 acquiring the first lock\n");
    heldLock->Acquire();
    printf("Lock has been acquired\n");
    printf("Wait (sleep) thread 1\n");
    testCond->Wait(heldLock);
    printf("Thread 1 has woken up. This should not print\n");
    heldLock->Release();
}

void
signalLockCall2(int param){
    printf("Thread 2 acquiring a secondary lock\n");
    secondaryLock->Acquire();
    printf("Secondary lock has been acquired\n");
    printf("Attemping to call signal with different lock\n");
    testCond->Wait(heldLock);
    printf("This should not print");
    secondaryLock->Release();

}

void
signalLockCall(){
    DEBUG('t',"Entering signalLockCall");

    heldLock = new Lock("heldLock");
    secondaryLock = new Lock("secondaryLock");
    testCond = new Condition("testCond");

    Thread *t = new Thread("one");
    t->setPriority(2);
    t->Fork(signalLockCall1,0);
    t = new Thread("two");
    t->setPriority(1);
    t->Fork(signalLockCall2,0);  
}

//----------------------------------------------------------------------
// deleteWaitLock
// Delete a lock that has threads on the wait queue
//----------------------------------------------------------------------
void
deleteWaitLock1(int param){
    printf("Acquiring lock once\n");
    heldLock->Acquire();
    printf("Acquired lock once. Yielding to different thread\n");
    currentThread->Yield();
    printf("Releasing first lock\n");
    heldLock->Release();
    printf("Released the first lock\n");
}

void
deleteWaitLock2(int param){
    printf("Attempting to aquire the same lock again\n");
    heldLock->Acquire();
    printf("Acquired lock twice. Yielding to a different thread\n");
    currentThread->Yield();
    printf("Attempting to release the second lock (same lock)\n");
    heldLock->Release();
    printf("Released the heldLock the second time around\n");
}

void
deleteWaitLock3(int param){
    printf("Attempt to delete a lock with waiting threads\n");
    delete heldLock;
    printf("This shouldn't print\n");
}

void
deleteWaitLock(){
    DEBUG('t',"Entering deleteWaitLock");

    heldLock = new Lock("heldLock");

    Thread *t = new Thread("one");
    t->setPriority(3);
    t->Fork(deleteWaitLock1, 0);
    t = new Thread("two");
    t->setPriority(2);
    t->Fork(deleteWaitLock2, 0);
    t = new Thread("three");
    t->setPriority(1);
    t->Fork(deleteWaitLock3, 0);
}


//----------------------------------------------------------------------
// deleteWaitCV
// Delete a condition variable that has threads on the wait list
//----------------------------------------------------------------------
void
deleteWaitCV1(int param){
    printf("Thread 1 acquiring the lock\n");
    heldLock->Acquire();
    printf("Lock has been acquired\n");
    printf("Wait (sleep) thread 1\n");
    testCond->Wait(heldLock);
    printf("Thread 1 has woken up\n");
    heldLock->Release();
}

void
deleteWaitCV2(int param){
    printf("Thread 2 acquiring the lock\n");
    heldLock->Acquire();
    printf("Lock has been acquired\n");
    printf("Attemping to delete the condition variable with threads on waitlist\n");
    delete testCond;
    printf("This should not print \n");
}

void
deleteWaitCV(){
    DEBUG('t',"Entering deleteWaitCV");

    heldLock = new Lock("heldLock");
    testCond = new Condition("testCond");

    Thread *t = new Thread("one");
    t->setPriority(2);
    t->Fork(deleteWaitCV1,0);

    t = new Thread("two");
    t->setPriority(1);
    t->Fork(deleteWaitCV2,0);  
}


// helper function to test Jion;
void printfSomeThing(){
    int i;
  for (i = 0; i <= 5; i++) {
    currentThread->Yield();
    printf(" hey here is a test %d\n" , i);
    }
}


//__________________________________________________________________________
//JointTest
//
// a thread that will be joined only is destroyed once Join has been called on it
   void parentFunction() {
    Thread *child = new Thread("parent fun", 1);

    printf("Testing 1st test. Child will be deleted after joined is called\n");
    currentThread->Yield();
    currentThread->Yield();
    currentThread->Yield();
    printf("calling Join()......\n");
    child->Fork((VoidFunctionPtr)printfSomeThing, 1);
    child->Join();   // child call after Join will not inmmedetely be deleted.
    if (child != NULL) {
        printf("If you see me, you passed.\n");
    }
}



//----------------------------------------------------------------------
// NoBlockingAfterChildFinish
//if a parent calls Join on a child and the child has finished executing, 
//the parent does not block
// 
//----------------------------------------------------------------------
void parentFunction7() {
    Thread *child = new Thread("parent fun", 1);
    printf("Testing 6th test. 2nd time join() from the same thread - seg fault.\n");
    currentThread->Yield();
    currentThread->Yield();
    currentThread->Yield();
    printf("First time calling Join() is FINE.\n");
    child->Join();
    printf("About to call the same thread to join 2nd time.\n");
    printf("You should see the program ended and could easily lead to a segmentation fault.\n");
    child->Join();
    printf("Error, YOU SHOULD NOT SEE ME.\n");
}

void childFunction7()
{
  printf("I am child thread.\n");
  currentThread->Yield();
}


void NoBlockingAfterChildFinish() {
    Thread* parent = new Thread("parent", 0); // parent will not be joined
    Thread* child = new Thread("child", 1); // child will be joined

    parent->Fork((VoidFunctionPtr) parentFunction7, (int)child);
    child->Fork((VoidFunctionPtr) childFunction7, 0);

    printf("If you see this, thread test 7 is DONE.\n");
}

//----------------------------------------------------------------------

// mailTest
// Tests the mailbox implementation
// Sends one message and recieves one message
//----------------------------------------------------------------------
Mailbox *mBox = NULL;
int theMessage;
int *theMessagePtr=&theMessage;

void mailTest1(int param){
    printf("Thread 1 Send a message\n");
    mBox->Send(12);

    printf("Thread 1 Finish send\n");
}

void mailTest2(int param){
    printf("Thread 2 Try receiving the message\n");
    mBox->Receive(theMessagePtr);

    printf("Thread 2 The received message was: %i\n",theMessage);
}


void mailTest(){
    DEBUG('t',"Entering mailTest");

    printf("Create a new mailbox\n");
    mBox = new Mailbox();

    Thread *t = new Thread("one");
    t->setPriority(2);
    t->Fork(mailTest1, 0);
    t = new Thread("two");
    t->setPriority(1);
    t->Fork(mailTest2, 0);
}

//----------------------------------------------------------------------
// mailTestReverse
// Tests the mailbox implementation
// Receives one message before sending one message
//----------------------------------------------------------------------
void mailTestReverse1(int param){
    printf("Thread 1 Try receiving the message\n");
    mBox->Receive(theMessagePtr);

    printf("Thread 1 The received message was: %i\n",theMessage);
}

void mailTestReverse2(int param){
    printf("Thread 2 Send a message\n");
    mBox->Send(24);

    printf("Thread 2 Finish send\n");
}


void mailTestReverse(){
    DEBUG('t',"Entering mailTestReverse");

    printf("Create a new mailbox\n");
    mBox = new Mailbox();

    Thread *t = new Thread("one");
    t->setPriority(2);
    t->Fork(mailTestReverse1, 0);
    t = new Thread("two");
    t->setPriority(1);
    t->Fork(mailTestReverse2, 0);
}


//----------------------------------------------------------------------
// mailSendTest
// Tests the mailbox implementation
// Sends one message and see if it waits
//----------------------------------------------------------------------

void mailSendTest1(int param){
    printf("Send a message\n");
    mBox->Send(12);

    printf("Finished send. This message should not appear\n");
}

void mailSendTest(){
    DEBUG('t',"Entering mailSendTest");

    printf("Create a new mailbox\n");
    mBox = new Mailbox();

    Thread *t = new Thread("one");
    t->Fork(mailSendTest1, 0);
}


//----------------------------------------------------------------------
// mailReceiveTest
// Tests the mailbox implementation
// Attempts to receive a message without a send call and sees if it
// returns or waits
//----------------------------------------------------------------------

void mailReceiveTest1(int param){
    printf("Receive a message\n");
    mBox->Receive(theMessagePtr);

    printf("Finished receive. This message should not appear\n");
}

void mailReceiveTest(){
    DEBUG('t',"Entering mailReceiveTest");

    printf("Create a new mailbox\n");
    mBox = new Mailbox();

    Thread *t = new Thread("one");
    t->Fork(mailReceiveTest1, 0);
}

//----------------------------------------------------------------------
// mailMultiTest
// Sends multiple messages and recieves the messages
//----------------------------------------------------------------------

void mailMultiTest1(int param){
    printf("Thread 1 is sending the message 11\n");
    mBox->Send(11);

    printf("Thread 1 finished sending the message\n");
}

void mailMultiTest2(int param){
    printf("Thread 2 is sending the message 12\n");
    mBox->Send(12);

    printf("Thread 2 finished sending the message\n");
}

void mailMultiTest3(int param){
    printf("Thread 3 is sending the message 13\n");
    mBox->Send(13);

    printf("Thread 3 finished sending the message\n");
}

void mailMultiTest4(int param){
    printf("Receive first message from mailbox\n");
    mBox->Receive(theMessagePtr);

    printf("First recieve finished receiving the message with value: %i\n", theMessage);
}

void mailMultiTest5(int param){
    printf("Receive first message from mailbox\n");
    mBox->Receive(theMessagePtr);

    printf("Second recieve finished receiving the message with value: %i\n", theMessage);
}

void mailMultiTest6(int param){
    printf("Receive first message from mailbox\n");
    mBox->Receive(theMessagePtr);

    printf("Third recieve finished receiving the message with value: %i\n", theMessage);
}
void mailMultiTest(){
    DEBUG('t',"Entering mailMultiTest");

    printf("Create a new mailbox\n");
    mBox = new Mailbox();

    Thread *t = new Thread("one");
    t->setPriority(6);
    t->Fork(mailMultiTest1, 0);
    t = new Thread("two");
    t->setPriority(5);
    t->Fork(mailMultiTest2, 0);
    t = new Thread("three");
    t->setPriority(4);
    t->Fork(mailMultiTest3, 0);
    t = new Thread("four");
    t->setPriority(3);
    t->Fork(mailMultiTest4, 0);
    t = new Thread("five");
    t->setPriority(2);
    t->Fork(mailMultiTest5, 0);
    t = new Thread("six");
    t->setPriority(1);
    t->Fork(mailMultiTest6, 0);

}

//----------------------------------------------------------------------
// WhaleTest
// Tests to see if the whales mate correctly
//----------------------------------------------------------------------
Whale *theWhale = NULL;

void WhaleTest1(int param){
    printf("Calling Male \n");
    theWhale->Male();
    printf("Thread 1 returned\n");

}
void WhaleTest2(int param){
    printf("Calling Female \n");
    theWhale->Female();
    printf("Thread 2 returned\n");
}
void WhaleTest3(int param){
    printf("Calling Matchmaker \n");
    theWhale->Matchmaker();
    printf("Thread 3 returned\n");
}
void WhaleTest4(int param){
    printf("Calling Matchmaker2 \n");
    theWhale->Matchmaker();
    printf("Thread 4 returned\n");
}
void WhaleTest(){
    DEBUG('t',"Entering WhaleTest");

    printf("Create a new whale class\n");
    theWhale = new Whale("theWhale");

    Thread *t = new Thread("one");
    t->setPriority(6);
    t->Fork(WhaleTest1, 0);
    t = new Thread("two");
    t->setPriority(5);
    t->Fork(WhaleTest3, 0);
    t = new Thread("three");
    t->setPriority(4);
    t->Fork(WhaleTest4, 0);
    t = new Thread("four");
    t->setPriority(3);
    t->Fork(WhaleTest2, 0);
    t = new Thread("five");
    t->setPriority(2);
    t->Fork(WhaleTest1, 0);
    t = new Thread("six");
    t->setPriority(1);
    t->Fork(WhaleTest2, 0);
}

//----------------------------------------------------------------------
// testPriority
// Tests to see when a thread is added to the readyList, the thread
// inserts into the readyList in sorted order.
//----------------------------------------------------------------------
void t1(int param) {
    printf("Thread with priority 3 ran.\n");
}
void t2(int param) {
    printf("Thread with priority -2 ran.\n");
}
void t3(int param) {
    printf("Thread with priority 1 ran.\n");
}
void t4(int param) {
    printf("Thread with priority 1v2 ran.\n");
}
void t5(int param) {
    printf("Thread with priority 4 ran.\n");
}
void testPrioritySort() {
    Thread *t = new Thread("3");
    t->setPriority(3);
    t->Fork(t1,0);
    t = new Thread("-2");
    t->setPriority(-2);
    t->Fork(t2,0);
    t = new Thread("1");
    t->setPriority(1);
    t->Fork(t3,0);
    t = new Thread("1v2");
    t->setPriority(1);
    t->Fork(t4,0);
    t = new Thread("4");
    t->setPriority(4);
    t->Fork(t5,0);
}

//----------------------------------------------------------------------
// testContextSwitchCaseOne(Two,Three)
// Tests to see that on a context switch, if there is a higher priority
// thread at the head of the ready list, the higher priority thread will
// run. If the thread at the head of the ready list has the same
// priority, then context switch still happens, and the given thread is
// prepended to the readylist.
//
// 3 CASES:
// next = 4  rl = [3;1] - when the priority at the head is lower 
// next = 3  rl = [4;4] - when the priority at the head is higher
//                      - this case also shows that if the called thread
//                        has a priority lower than the head, in this case
//                        the second 4 in the rl, then it will sort itself
//                        by priority when inserting back into the rl.
// next = 4  rl = [4;1] - when the priority at the head is the same
//----------------------------------------------------------------------

void th1CaseOne(int param) {
    printf("th1 ran.\n");
    currentThread->Yield();
    printf("yield ignored because this thread has the highest priority, success.\n");
}
void th1(int param) {
    printf("th1 ran.\n");
}
void th2(int param) {
    printf("th2 ran.\n");
}
void th3(int param) {
    printf("th3 ran.\n");
}

void testContextSwitchCaseOne() {
    Thread *t = new Thread("4");
    t->setPriority(4);
    t->Fork(th1CaseOne,0);

    t = new Thread("3");
    t->setPriority(3);
    t->Fork(th2,0);

    t = new Thread("1");
    t->setPriority(1);
    t->Fork(th3,0);
    printf("Order should be th1, th2, th3\n");
}
void testContextSwitchCaseTwo() {
    Thread *t = new Thread("4");
    t->setPriority(4);
    t->Fork(th1,0);

    t = new Thread("4v2");
    t->setPriority(4);
    t->Fork(th2,0);

    t = new Thread("3");
    t->setPriority(5);
    t->Fork(th3,0);
    t->setPriority(3);
    printf("Order should be th1, th2, th3\n");
}
void testContextSwitchCaseThree() {
    Thread *t = new Thread("4");
    t->setPriority(4);
    t->Fork(th1,0);

    t = new Thread("4v2");
    t->setPriority(4);
    t->Fork(th2,0);

    t = new Thread("1");
    t->setPriority(1);
    t->Fork(th3,0);
    printf("Order should be th1, th2, th3\n");
}

//----------------------------------------------------------------------
// testNoSort
// Tests to see that a thread is not re-sorted when its priority is
// changed.
//----------------------------------------------------------------------
void thr1(int param) {
    printf("Thread with priority 4 ran.\n");
}
void thr2(int param) {
    printf("Thread with priority 4v2 ran.\n");
}
void thr3(int param) {
    printf("Thread with priority 3 ran.\n");
}

void testNoSort() {
    Thread *t = new Thread("4");
    t->setPriority(4);
    t->Fork(thr1,0);

    t = new Thread("4v2");
    t->setPriority(4);
    t->Fork(thr2,0);

    t = new Thread("5");
    t->setPriority(5);
    t->Fork(thr3,0);
    t->setPriority(3);
    printf("Changed thr3's priority to 3.\n");    
}

//----------------------------------------------------------------------
// testWaitPriorityLock, testWaitPriorityCV, testWaitPrioritySema
// Tests to see when threads are waiting for a lock, semaphore, or 
// condition variable, the highest priority waiting thread should be 
// woken up first.
//----------------------------------------------------------------------

Lock *prioLock = NULL;
void firstAcquireLock(int param) {
    printf("Acquiring lock for the first time.\n");    
    prioLock->Acquire();
    printf("Lock acquired.\n");
    printf("Yielding thread.\n");
    currentThread->setPriority(0);
    currentThread->Yield();
    printf("Releasing lock.\n");
    prioLock->Release();
}
void blockPrio1Lock(int param) {
    printf("Trying to acquire the lock with priority 1.\n");
    currentThread->setPriority(1);
    prioLock->Acquire();
    printf("Woke up from sleep priority 1. Fail.\n");
}
void blockPrio2Lock(int param) {
    printf("Trying to acquire the lock with priority 2.\n");
    currentThread->setPriority(2);
    prioLock->Acquire();
    printf("Woke up from sleep with priority 2. Success.\n");
}


void testWaitPriorityLock() {
    prioLock = new Lock("prioLock");

    Thread *t = new Thread("firstAcquire");
    t->setPriority(3);
    t->Fork(firstAcquireLock,0);

    t = new Thread("blockPrio1");
    t->setPriority(2);
    t->Fork(blockPrio1Lock,0);

    t = new Thread("blockPrio2");
    t->setPriority(1);
    t->Fork(blockPrio2Lock,0);
}


Lock *prioCondLock = NULL;
Condition *prioCond = NULL;
void firstWait(int param) {
    printf("Acquiring lock for the first time.\n");    
    prioCondLock->Acquire();
    printf("Lock acquired.\n");
    currentThread->setPriority(1);
    printf("Waiting with priority 1.\n");
    prioCond->Wait(prioCondLock);
    printf("Woke up from sleep with priority 1. Fail.\n");
}
void secondWait(int param) {
    prioCondLock->Acquire();
    printf("Lock acquired.\n");
    printf("Waiting with priority 2.\n");
    prioCond->Wait(prioCondLock);
    printf("Woke up from sleep with priority 2. Success.\n");
}
void signalPrio(int param) {
    prioCondLock->Acquire();
    printf("Lock acquired.\n");
    prioCond->Signal(prioCondLock);
    printf("Signaled.\n");
    prioCondLock->Release();
    printf("Lock released.\n");
}

void testWaitPriorityCV() {
    prioCondLock = new Lock("prioCondLock");
    prioCond = new Condition("prioCond");

    Thread *t = new Thread("firstWait");
    t->setPriority(3);
    t->Fork(firstWait,0);

    t = new Thread("secondWait");
    t->setPriority(2);
    t->Fork(secondWait,0);

    t = new Thread("signalPrio");
    t->setPriority(1);
    t->Fork(signalPrio,0);
}


Semaphore *prioSem = NULL;
void sleepSem1(int param) {
    printf("Thread goes to sleeps with priority 2.\n");
    currentThread->setPriority(2);
    prioSem->P();
}
void sleepSem2(int param) {
    printf("Thread goes to sleep with priority 3.\n");
    currentThread->setPriority(3);    
    prioSem->P();
    printf("Thread with priority 3 woke up. Success.\n");

}
void wakeSem(int param) {
    prioSem->V();    
}

void testWaitPrioritySema() {
    prioSem = new Semaphore("eight",1);

    Thread *t = new Thread("t1");
    t->setPriority(3);
    t->Fork(sleepSem1,0);

    t = new Thread("t2");
    t->setPriority(2);
    t->Fork(sleepSem2,0);

    t = new Thread("t3");
    t->setPriority(1);
    t->Fork(wakeSem,0);
}

//----------------------------------------------------------------------
// ThreadTest

// callSelfTest
//  thread does not call Join on itself,
//----------------------------------------------------------------------

void callSelfTest()
{
  printf("thread is calling it self.\n");
  currentThread->Join();
  printf("ERROR.!!!! You should call yourself\n");
}

//----------------------------------------------------------------------
// JointTest
// Join is only invoked on threads created to be joined
//----------------------------------------------------------------------

 Thread *child1 = new Thread("child1", 1); // be joined
 Thread *child2 = new Thread("child1", 0); // NOt be joined

 void notJoinedThreadTest() {
        child1->Fork((VoidFunctionPtr)printfSomeThing, 1);
    printf (" child1 is being joined\n");
    child1->Join();
    printf("child is being \n");
    printf (" child2 is being joined\n");
    child2->Fork((VoidFunctionPtr)printfSomeThing, 0);
    printf (" should not print this statement");

 }
  


//----------------------------------------------------------------------
// NoForkJoin()
// Join is only called on a thread that has forked.
// it should be abort.
//----------------------------------------------------------------------
;
void NoForkJoin () {
    Thread *child = new Thread("no fork", 1);
    child->Join();
    printf("this statement should not be print because child has not fork yets\n");
}

//----------------------------------------------------------------------
// JointTest
//  Join is not called more than once on a thread
//----------------------------------------------------------------------

void callJoinTwice() {
    printf("create a Join\n");
    Thread *child = new Thread("callTWice\n", 1);

    child->Fork((VoidFunctionPtr)printfSomeThing, 0);
    
    child->Join();
    printf("called join first time\n");

    child->Join();
    printf("called join second time\n");
    printf("this statement should not be print because join cannot be called more then noce\n");

}


//----------------------------------------------------------------------
// JointTest

//  Invoke a test routine.
//----------------------------------------------------------------------



void
ThreadTest()
{
    switch (testnum) {
    case 1:
    ThreadTest1(); break;
    case 2:
    LockTest1(); break;
    case 3:
    acquireSameLock(); break;
    case 4:
    acquireSameLock2(); break;
    case 5:
    releaseUnheldLock(); break;
    case 6:
    deleteUnheldLock(); break;
    case 7:
    deleteHeldLock(); break;
    case 8:
    waitCondUnheldLock(); break;
    case 9:
    signalThread(); break;
    case 10:
    broadcastThread(); break;
    case 11:
    noWaitSignal(); break;
    case 12:
    noWaitBroadcast(); break;
    case 13:
    signalLockCall(); break;
    case 14:
    deleteWaitLock(); break;
    case 15:
    deleteWaitCV(); break;

    case 16:
    mailTest(); break;
    case 17:
    mailTestReverse(); break;
    case 18:
    mailSendTest(); break;
    case 19:
    mailReceiveTest(); break;
    case 20:
    mailMultiTest(); break;
    case 21:
    WhaleTest(); break;

    //Part 4 Tests
    case 22:
    testPrioritySort(); break;
    case 23:
    testContextSwitchCaseOne(); break;
    case 24:
    testContextSwitchCaseTwo(); break;
    case 25:
    testContextSwitchCaseThree(); break;
    case 26:
    testNoSort(); break;
    case 27:
    testWaitPriorityLock(); break;
    case 28:
    testWaitPriorityCV(); break;
    case 29:
    testWaitPrioritySema(); break;

    case 30: 
    parentFunction(); break;
    case 31:
    callSelfTest(); break;
    case 32:
    notJoinedThreadTest();
    break;
    case 33:
    NoForkJoin();  break;
    case 34:
    callJoinTwice(); break;
    case 35: 
    NoBlockingAfterChildFinish();
    break;



    default:
    printf("No test specified.\n");
    break;
    }
}