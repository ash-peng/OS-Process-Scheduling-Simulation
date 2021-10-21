#ifndef _SIMULATOR_H_
#define _SIMULATOR_H_
#include "list.h"
#include <stdbool.h>

struct semaphore{
    int value;
    List* qSem;
    bool used;
};
typedef struct semaphore semaphore;

// Initialize ready queues, blocked queues, semaphores, and the "init" process.
void  start();

// Reads user command and performs wanted operation, before system end is triggered.
void  read_command();

// Create process. If no other process is running (except "init"), make it run.
// Else, put it in ready queue of the specified priority.
// Report success/failure and the PID generated.
void  Create(int priority);

// Copy current process and put it into the ready queue.
// Report success/failure and the PID generated.
void  Fork();

// Kill named process, remove it from the system.
// Report success or failure.
void  Kill(int pid);

// Kill current process, remove it from system.
// Report success or failure.
void  Exit();

// Time quantum for currently running process is up.
// Stop currently running process and put it back into ready queue;
// NOTE: if possible, LOWER its priority by one (to prevent it from hogging the CPU).
// Pick the next current process from ready queue, if any.
// If none, switch current process to "init".
void  Quantum();

// Sends a message to specified process,
// puts current process into the blocked queue to wait for reply.
// If the specified process had done a Receive before, it unblocks the said process and puts it into ready queue.
// Next time the receiver runs, the msg along with sender's PID will be printed.
void  Send(int pid, char* msg);

// Makes the current process do a Receive. Which means:
// If there is already a message sent to it, receive it (i.e. print it and clear the space.)
// Else, put process into the blocked queue to wait for a "Send".
void  Receive();

// Delivers reply to sender, and unblocks the sender.
void  Reply(int pid, char* msg);

// Initializes the named semaphore with the value given.
// ID's can take a value from 0 to 4.
// Can only be done once per semaphore.
void  New_sem(int sem_ID, int init_val);

// Executes P operation on named semaphore.
void  SemP(int sem_ID);

// Executes V operation on named semaphore.
void  SemV(int sem_ID);

// Prints all the information of a process (i.e. PCB).
void  Procinfo(int pid);

// Displays all process queues and their contents.
void  Totalinfo();

// Function to print the processes of a given queue, used by Totalinfo().
void  display_queue(List* queue);

// Selects a process from the ready queues based on priority,
// and sets it to running.
// If there is no process ready, switch to "init".
// If the newly running process has a proc_msg to print, print it.
void  switch_current_process();

// Finds process by searching input PID through all queues.
// Returns a pointer to the queue with process as the current item;
// Returns NULL if process not found.
List* search_process(int pid);

// Function to compare integers, used by search_process().
bool  comp_int(void* pItem, void* pComparisonArg);

// Free all the ready and blocked queues, as a part of clean up.
void  cleanup();

// Function required to invoke List_free() in cleanup().
// Does nothing here because the freeing of PCBs is handled by PCB_free().
void  free_item(void *pItem);


#endif