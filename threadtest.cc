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
    printf("Attempting to acquire the lock for the second time\n");
    sameLock->Acquire();
    printf("Acquired the lock the second time. This should not print\n");
}
void
acquireSameLock2(){
    DEBUG('t', "Entering acquireSameLock2");

    sameLock = new Lock("sameLock");

    Thread *t = new Thread("one");
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
    t->Fork(signalThread1,0);  
    t = new Thread("two");
    t->Fork(signalThread2,0);
    t = new Thread("three");
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
    t->Fork(broadcastThread1,0);  
    t = new Thread("two");
    t->Fork(broadcastThread2,0);
    t = new Thread("three");
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
    t->Fork(signalLockCall1,0);
    t = new Thread("two");
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
    t->Fork(deleteWaitLock1, 0);
    t = new Thread("two");
    t->Fork(deleteWaitLock2, 0);
    t = new Thread("three");
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
    t->Fork(deleteWaitCV1,0);
    t = new Thread("two");
    t->Fork(deleteWaitCV2,0);  
}

//----------------------------------------------------------------------
// ThreadTest
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
    default:
    printf("No test specified.\n");
    break;
    }
}