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
        queue->SortedInsert((void *)currentThread, currentThread->getPriority()*(-1));	// so go to sleep
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
    // Make sure that the Lock is not held and the queue is empty
    // before allowing deletion of a lock.
    ASSERT(!held);
    ASSERT(queue->IsEmpty());
    delete queue;
}
void Lock::Acquire() {
    // disable interrupts
    IntStatus oldLevel = interrupt->SetLevel(IntOff); 

    // While the lock is already held
    while (held) {           
        queue->SortedInsert((void *)currentThread, currentThread->getPriority()*(-1));   // Go to sleep
        currentThread->Sleep();
    }

    // Once Lock is free, make current thread the lock owner and
    // change the lock's status to held.
    lockOwner = currentThread;
    held = true;

    (void) interrupt->SetLevel(oldLevel);   // re-enable interrupts
}
void Lock::Release() {
    // Make sure that the lock is held by the current thread.
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
    // Check to see if the waiting list is empty before allowing
    // deletion of the condition variable
    ASSERT(waitingList->IsEmpty());
    delete waitingList;
}
void Condition::Wait(Lock* conditionLock) {
    ASSERT(conditionLock->isHeldByCurrentThread());
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    // Release the lock
    conditionLock->Release();
    // Place the calling thread on the condition variable's waiting list
    waitingList->SortedInsert((void *)currentThread, currentThread->getPriority()*(-1));
    // Suspend the execution of the calling thread
    currentThread->Sleep();

    // When it wakes up, reacquire the lock
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

        // Calls one thread off the condition variable's witing list
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
        // Used for testing purposed to notify that the 
        // waiting list was empty.
        printf("There were no waiters\n");
    }
}

Mailbox::Mailbox(){
    // Initialize Mailbox's private variables.
    mailboxLock = new Lock("Mailbox_lock");
    send_msg = new Condition("Mailbox_send");
    receive_msg = new Condition("Mailbox_receive");
    msg = new List;
    senders = 0;
    receivers = 0;
}

Mailbox::~Mailbox(){
    delete mailboxLock;
    delete send_msg;
    delete receive_msg;
}

void Mailbox::Send(int message){
    // Acquire lock for mutual exclusion
    mailboxLock->Acquire();
    // Increment the amount of senders
    senders++;
    // If there are no receivers waiting, place the thread to sleep
    if(receivers==0){
        //printf("Send is going to sleep\n");
        send_msg->Wait(mailboxLock);
        //printf("Send woke up\n");
    }

    // Decrement the number of receivers since we will be sending
    // a message to a receiver that is waiting
    receivers--;
    // printf("Sending the message\n");

    // Send the message, and signal the Receive condition variable
    // to wake up the sleeping receive thread
    msg->Append((void *) message);
    receive_msg->Signal(mailboxLock);

    // Release lock
    mailboxLock->Release();
}


void Mailbox::Receive(int *message){
    // Acquire lock for mutual exclusion
    mailboxLock->Acquire();

    // Increment the amount of receivers available
    receivers++;

    // Wake up the send thread if there is a send waiting
    // for a receive.
    send_msg->Signal(mailboxLock);

    // If there is no senders waiting, place the receive to sleep.
    receive_msg->Wait(mailboxLock);

    // Once awaken, do a second check (this is for when receive is
    // called before send)
    if(senders==0){
        //printf("Receive is going to sleep\n");
        receive_msg->Wait(mailboxLock);
    }

    // Decrement the amount of senders since receive will be
    // receive a message from the sender
    senders--;

    // Receive the message, change the value of the passed
    // message pointer, and release the lock.
    printf("Receiving the message\n");
    *message = (int) (msg->Remove());
    mailboxLock->Release();
}

Whale::Whale(char* debugName){
    // Here we initialize the private variables of Whale.
    name = debugName;

    // Condition variables
    cv_male = new Condition("male_whale");
    cv_female = new Condition("female_whale");
    cv_matchmaker = new Condition("matchmaker_whale");

    whaleLock = new Lock("whale_lock");

    // Number of males, females, and matchmakers available.
    males = 0;
    females = 0;
    matchmakers = 0;
}

Whale::~Whale(){
    delete cv_male;
    delete cv_female;
    delete cv_matchmaker;

    delete whaleLock;
}

/* The code structure and behavior is almost identical between the
   Male, Female, and Matchmaker functions. Because of this, we will
   only comment the Male function. */
void Whale::Male(){
    // Acquire a lock to achieve mutual exclusion
    whaleLock->Acquire();

    // Incremement the number of males available.
    males++;

    // If there already exists at least 1 female AND matchmaker,
    // a mating can occur.
    if(females>0 && matchmakers>0){
        // Since the mating occurs, decrement the number of
        // females, males, and matchmakers available.
        females--;
        matchmakers--;
        males--;

        // Wake up sleeping female and matchmaker threads.
        cv_female->Signal(whaleLock);
        cv_matchmaker->Signal(whaleLock);
        //printf("**Mating occured**\n");
    }

    // If conditions aren't met for mating, sleep the male thread
    else{
        cv_male->Wait(whaleLock);
    }

    // Release the lock
    whaleLock->Release();
}

void Whale::Female(){
    whaleLock->Acquire();
    females++;
    if(males>0 && matchmakers>0){
        females--;
        matchmakers--;
        males--;
        cv_male->Signal(whaleLock);
        cv_matchmaker->Signal(whaleLock);
        printf("**Mating occured**\n");
    }
    else{
        cv_female->Wait(whaleLock);
    }

    whaleLock->Release();
}

void Whale::Matchmaker(){
    whaleLock->Acquire();
    matchmakers++;
    if(males>0 && females>0){
        females--;
        matchmakers--;
        males--;
        cv_male->Signal(whaleLock);
        cv_female->Signal(whaleLock);
        printf("**Mating occured**\n");
    }
    else{
        cv_matchmaker->Wait(whaleLock);
    }
    whaleLock->Release();
}
