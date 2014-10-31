// synch.cc
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

    while (value == 0) { 			// semaphore not available
        queue->Append((void *)currentThread);	// so go to sleep
        currentThread->Sleep();
    }
    value--; 					// semaphore available,
    // consume its value

    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
        scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments
// Note -- without a correct implementation of Condition::Wait(),
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {
    name = debugName;
    queue = new List;
    held = false;
}
Lock::~Lock() {
    ASSERT(!held);
    ASSERT(queue->IsEmpty());
    delete queue;
}
void Lock::Acquire() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts

    while (held) {            // semaphore not available
        queue->Append((void *)currentThread);   // so go to sleep
        currentThread->Sleep();
    }
    lockOwner = currentThread;
    held = true;

    (void) interrupt->SetLevel(oldLevel);   // re-enable interrupts
}
void Lock::Release() {
    
    ASSERT(isHeldByCurrentThread());
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts

    thread = (Thread *)queue->Remove();
    if (thread != NULL)    // make thread ready, consuming the V immediately
        scheduler->ReadyToRun(thread);
    lockOwner = NULL;
    held=false;

    (void) interrupt->SetLevel(oldLevel); // re-enable interrupts
}

bool Lock::isHeldByCurrentThread(){
    return (currentThread==lockOwner);

}

Condition::Condition(char* debugName) {
    name = debugName;
    waitingList = new List;
}
Condition::~Condition() {
    ASSERT(waitingList->IsEmpty());
    delete waitingList;
}
void Condition::Wait(Lock* conditionLock) {
    ASSERT(conditionLock->isHeldByCurrentThread());
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    // Release the lock
    conditionLock->Release();
    // Place the calling thread on the condition variable's waiting list
    waitingList->Append((void *)currentThread);
    // Suspend the execution of the calling thread
    currentThread->Sleep();

    // When it wakes up, require the lock
    conditionLock->Acquire();

    // Re-enable interrupts
    (void) interrupt->SetLevel(oldLevel);
}
void Condition::Signal(Lock* conditionLock) {
    // Check to see if the list is empty
    if(!waitingList->IsEmpty()){
        ASSERT(conditionLock->isHeldByCurrentThread());

        Thread *thread;

        // Disable interrupts
        IntStatus oldLevel = interrupt->SetLevel(IntOff);

        // Calls one thread off the conditionv ariable's witing list
        // And marks it as eligible to run
        thread = (Thread *)waitingList->Remove();
        if (thread != NULL)    // make thread ready, consuming the V immediately
            scheduler->ReadyToRun(thread);

        // Re-enable interrupts
        (void) interrupt->SetLevel(oldLevel);
    }
    else{
        printf("There were no waiters\n");
    }
}
void Condition::Broadcast(Lock* conditionLock) {
    // Check to see if the list is empty
    if(!waitingList->IsEmpty()){
        ASSERT(conditionLock->isHeldByCurrentThread());

        Thread *thread;

        // Disable interrupts
        IntStatus oldLevel = interrupt->SetLevel(IntOff);

        // Take all threads off the condition variable's waiting list
        // and marks them as eligible to run.
        thread = (Thread *)waitingList->Remove();
        while(thread != NULL){
            scheduler->ReadyToRun(thread);
            thread = (Thread *)waitingList->Remove();
        }

        // Re-enable the interrupts
        (void) interrupt->SetLevel(oldLevel);
    }
    else{
        printf("There were no waiters\n");
    }
}

Mailbox::Mailbox(){
    mailboxLock = new Lock("Mailbox_lock");
    send_msg = new Condition("Mailbox_send");
    receive_msg = new Condition("Mailbox_receive");
    msg = new List;
}

Mailbox::~Mailbox(){
    delete mailboxLock;
    delete send_msg;
    delete receive_msg;
}

void Mailbox::Send(int message){
    mailboxLock->Acquire();
    receive_msg->Signal(mailboxLock);
    send_msg->Wait(mailboxLock);
    msg->Append((void *) message);
    receive_msg->Signal(mailboxLock);
    mailboxLock->Release();
}


void Mailbox::Receive(int *message){
    mailboxLock->Acquire();
    send_msg->Signal(mailboxLock);
    receive_msg->Wait(mailboxLock);
    *message = (int) (msg->Remove());
    send_msg->Signal(mailboxLock);
    mailboxLock->Release();
}



