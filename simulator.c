#include "simulator.h"
#include "PCB.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

static List* qHigh;
static List* qMid;
static List* qLow;
static List* qReceiver;
static List* qSender;
static semaphore semaphores[5];
static PCB* init;
static PCB* current;
static bool systemEnd = false;


int main(){
    printf("System is starting...\n");
    start();
    read_command();
    cleanup();
    printf("System is ending...\n");
    return 0;
}


// Initialize ready queues, blocked queues, semaphores, and the "init" process.
void start(){
    // Three ready queues, one for each priority
    qHigh = List_create();
    qMid  = List_create();
    qLow  = List_create();

    // A queue of blocked processes that has done a "Receive"
    // and is waiting for a "Send"
    qReceiver = List_create();

    // A queue of blocked processes that has done a "Send"
    // and is waiting for a "Reply"
    qSender = List_create();

    if (!(qHigh && qMid && qLow && qReceiver && qSender)){
        printf("Queue creation error.\n");
        return;
    } else{
        printf("Successfully created 3 ready queues and 2 blocked queues.\n");
    }

    // Five semaphores, IDed 0 to 4
    for (int i = 0; i < 5; i++){
        semaphores[i].value = -1;
        semaphores[i].qSem = List_create();
        semaphores[i].used = false;
    }
    if (!(semaphores[0].qSem && semaphores[1].qSem && semaphores[2].qSem &&
        semaphores[3].qSem && semaphores[4].qSem)){
        printf("Semaphore creation error.\n");
        return;
    } else{
        printf("Successfully created 5 semaphores and their waiting queues.\n");
    }
    
    // The "init" process, which stands out on its own
    // running as long as there is nothing in the ready queue...
    init = PCB_create(-1); // priority: -1 (isn't part of ready lists)
    if (init == NULL){
        printf("Init creation failure. Returning...\n");
        return;
    }
    init->PID = 0;
    init->state = RUNNING;
    current = init;
    printf("Currently running process: \"init\"\n");
}


// Reads user command and performs wanted operation, before system end is triggered.
void read_command(){
    char command;
    printf("\nPlease select command:\n"
        " (C): Create process\n"
        " (F): Fork process\n"
        " (K): Kill process\n"
        " (E): Exit current process\n"
        " (Q): Signify time quantum end\n"
        " (S): Send message to designated process\n"
        " (R): Receive message\n"
        " (Y): Reply to received message\n"
        " (N): Create new semaphore\n"
        " (P): Do P() operation on named semaphore\n"
        " (V): Do V() operation on named semaphore\n"
        " (I): Print process status info\n"
        " (T): Print total info on all managed queues\n");
    while (systemEnd == false){
        printf("> ");
        scanf(" %c", &command);
        
        switch(command){
            case 'C':
            case 'c':;{
                int priority;
                printf("Getting ready to create new process...\n");
                printf("Please enter process priority: 0-high; 1-mid; 2-low\n");
                printf("> ");
                scanf("%d", &priority);
                if (priority == 0 || priority == 1 || priority == 2){
                    Create(priority);
                } else{
                    printf("Input error. Back to main menu...\n");
                }
                break;
            }
            case 'F':
            case 'f':{
                Fork();
                break;
            }
            case 'K':
            case 'k':;{
                int pid;
                printf("Getting ready to kill a process...\n");
                printf("Please enter PID of process to kill:\n");
                printf("> ");
                scanf("%d", &pid);
                Kill(pid);

                break;
            }
            case 'E':
            case 'e':{
                Exit();
                break;
            }
            case 'Q':
            case 'q':{
                Quantum();
                break;
            }
            case 'S':
            case 's':;{
                int pid;
                char msg[MSG_MAX];
                printf("Please enter PID of the process to send to:\n");
                printf("> ");
                scanf("%d", &pid);
                printf("Please enter message:\n");
                printf("> ");
                scanf(" %[^\n]%*c", msg);
                fflush(stdin);
                Send(pid, msg);
                
                break;
            }
            case 'R':
            case 'r':{
                Receive();
                
                break;
            }
            case 'Y':
            case 'y':;{
                int pid;
                char msg[MSG_MAX];
                printf("Please enter PID of the process to reply to:\n");
                printf("> ");
                scanf("%d", &pid);
                printf("Please enter message:\n");
                printf("> ");
                scanf(" %[^\n]%*c", msg);
                fflush(stdin);
                Reply(pid, msg);
                
                break;
            }
            case 'N':
            case 'n':;{
                int semaphore = -1;
                int init_val = -1;
                printf("Getting ready to create new semaphore...\n");
                while (semaphore < 0 || semaphore > 4){
                    printf("Please enter semaphore ID (0-4):\n");
                    printf("> ");
                    scanf("%d", &semaphore);
                }
                while (init_val < 0){
                    printf("Please enter initial value of semaphore (non-negative):\n");
                    printf("> ");
                    scanf("%d", &init_val);
                }
                New_sem(semaphore, init_val);

                break;
            }
            case 'P':
            case 'p':;{
                int semaphore = -1;
                while (semaphore < 0 || semaphore > 4){
                    printf("Please enter semaphore ID:\n");
                    printf("> ");
                    scanf("%d", &semaphore);
                }
                SemP(semaphore);

                break;
            }
            case 'V':
            case 'v':;{
                int semaphore = -1;
                while (semaphore < 0 || semaphore > 4){
                    printf("Please enter semaphore ID:\n");
                    printf("> ");
                    scanf("%d", &semaphore);
                }
                SemV(semaphore);

                break;
            }
            case 'I':
            case 'i':;{
                int pid;
                printf("Getting ready to retrieve process information...\n");
                printf("Please enter PID of process:\n");
                printf("> ");
                scanf("%d", &pid);
                Procinfo(pid);

                break;
            }
            case 'T':
            case 't':{
                Totalinfo();
                
                break;
            }
            default:{
                printf("The command you entered is invalid.\n");
                printf("Please try again.\n");
            }
        }
    }
}


// Create process. If no other process is running (except "init"), make it run.
// Else, put it in ready queue of the specified priority.
// Report success/failure and the PID generated.
void Create(int priority){
    PCB* process = PCB_create(priority);
    if (process == NULL){
        printf("Failure in creating new process. Returning to menu...\n");
        return;
    }
    printf("Successfully created: Process %d\n", process->PID);
    // If init is running, make new process the running process.
    if (current == init){
        init->state = READY;
        process->state = RUNNING;
        current = process;
        printf("This process is now running.\n");
        return;
    }
    // If there is some other process running, put it into the ready queue
    if (process->priority == HIGH){
        List_append(qHigh, process);
    } else if (process->priority == MID){
        List_append(qMid, process);
    } else{
        List_append(qLow, process);
    }
}


// Copy current process and put it into the ready queue.
// Report success/failure and the PID generated.
void Fork(){
    if (current == init){
        printf("Failure to fork process, because the current process is \"init\".\n"
            "Returning...\n");
        return;
    }
    PCB* forked = PCB_create(current->priority);
    forked->from_PID = current->from_PID;
    strncpy(forked->msg_to_receive, current->msg_to_receive, MSG_MAX);
    //(forked->msg_to_receive)[MSG_MAX] = '\0';
    strncpy(forked->proc_message, current->proc_message, PROC_MSG_MAX);
    //(forked->proc_message)[PROC_MSG_MAX] = '\0';
    if (forked->priority == HIGH){
        List_append(qHigh, forked);
    } else if (forked->priority == MID){
        List_append(qMid, forked);
    } else{
        List_append(qLow, forked);
    }
    printf("Successfully forked current process.\nForked process PID: %d\n", forked->PID);
}


// Kill named process, remove it from the system.
// Report success or failure.
void Kill(int pid){
    // If the process is currently running, Exit() will handle that
    if (current->PID == pid){
        Exit();
        return;
    } else if (pid == 0){
        // If process is "init", and it is not running, killing it would fail
        printf("Failure to kill process: Process is \"init\" "
            "and other processes exist in system.\n");
        return;
    } else{
        // Process is not "init."
        // Search and find process to kill
        List* qFound = search_process(pid);
        if (qFound != NULL){
            // Special case: blocked on semaphore
            for (int i = 0; i < 5; i++){
                if (qFound == semaphores[i].qSem){
                    semaphores[i].value += 1;
                    printf("The process was blocked on Semaphore %d.\n", i);
                    printf("Semaphore value incremented by 1.\n");
                    printf("Semaphore %d now has value %d.\n", i, semaphores[i].value);
                }
            }
            // All cases
            PCB_free(List_remove(qFound));
            printf("Successfully killed Process %d from system.\n", pid);
        } else{
            printf("Failure to kill process: Process %d not found\n", pid);
        }
    }
}


// Kill current process, remove it from system.
// Report success or failure.
void Exit(){
    // If current process is init:
    // check all the ready and blocked queues for emptiness.
    // If empty, signify to end system.
    if (current == init){
        if (List_count(qHigh) || List_count(qLow) || List_count(qMid) ||
            List_count(qReceiver) || List_count(qSender) || List_count(semaphores[0].qSem) ||
            List_count(semaphores[1].qSem) || List_count(semaphores[2].qSem) ||
            List_count(semaphores[3].qSem) || List_count(semaphores[4].qSem)){
                printf("Failure to kill \"init\", because it is not the only process in system.\n");
        } else{
            printf("Verified that \"init\" is the last process in system.\n");
            PCB_free(init);
            systemEnd = true;
        }
        return;
    }
    // Current is not init:
    PCB_free(current);
    current = NULL;
    printf("Exited current process.\n");

    // pick next current from ready processes; switch to init if none
    switch_current_process();
}


// Time quantum for currently running process is up.
// Stop currently running process and put it back into ready queue;
// NOTE: if possible, LOWER its priority by one (to prevent it from hogging the CPU).
// Pick the next current process from ready queue, if any.
// If none, switch current process to "init".
void Quantum(){
    printf("Current quantum is reached.\n");
    if (current != init){
        if (current->priority == HIGH){
            List_append(qMid, current);
            current->priority = MID;
            printf("The priority of the current process is adjusted to MID.\n");
        } else if (current->priority == MID){
            List_append(qLow, current);
            current->priority = LOW;
            printf("The priority of the current process is adjusted to LOW.\n");
        } else{
            List_append(qLow, current);
        }
        current->state = READY;
        // pick next current from ready processes; switch to init if none
        switch_current_process();
    } else{
        printf("Current process is \"init\".\nNo action performed.\n");
    }
}


// Sends a message to specified process,
// puts current process into the blocked queue to wait for reply.
// If the specified process had done a Receive before, it unblocks the said process and puts it into ready queue.
// Next time the receiver runs, the msg along with sender's PID will be printed.
void Send(int pid, char* msg){
    PCB* dest = NULL;
    List* list_found = NULL;
    // Find receiver from input PID
    if (pid == current->PID){
        printf("A process cannot send a message to itself. Returning...\n");
        return;
    } else if (pid == 0){
        dest = init;
    } else{
        list_found = search_process(pid);
        if (list_found == NULL){
            printf("Failure to send message: PID not found in system.\n");
            return;
        } else{
            dest = List_curr(list_found);
        }
    }
    // Block current process to wait for reply.
    // (except when process is "init")
    if (current != init){
        List_append(qSender, current);
        current->state = BLOCKED;
        printf("Success: Process %d has sent message, and is now blocked for a reply.\n", current->PID);
    } else{
        printf("Success: Process %d has sent message; but it is not blocked, since it is \"init\".\n", current->PID);
    }    
    // If receiver is blocked because it had done a Receive before, unblock it,
    //      and prepare message to print for when it runs again.
    // Else, put the message in the msg_to_receive, and record the sender.
    if (list_found == qReceiver){
        // Prepare message
        strncpy(dest->msg_to_receive, msg, MSG_MAX);
        //(dest->msg_to_receive)[MSG_MAX] = '\0';
        sprintf(dest->proc_message, "Received Send() from Process %d, message is: %s\n", current->PID, dest->msg_to_receive);
        //dest->proc_message[PROC_MSG_MAX] = '\0';
        dest->msg_to_receive[0] = '\0'; // To avoid duplicates

        // Unblock receiver;
        // Place it into the ready queue (with priority adjusted)
        dest->state = READY;
        List_remove(qReceiver);
        printf("Process %d, who had been waiting for a Send, is now unblocked.\n", dest->PID);
        if (dest->priority == HIGH){
            List_append(qHigh, dest);
        } else if (dest->priority == MID){
            List_append(qHigh, dest);
            dest->priority = HIGH;
            printf("Its priority has been raised to HIGH.\n");
        } else{
            List_append(qMid, dest);
            dest->priority = MID;
            printf("Its priority has been raised to MID.\n");
        }
        // If current is init, it has to give its place to the unblocked receiver.
        // If current is not init, it has already been blocked.
        // Either way, a new process has to run.
        switch_current_process();
    } else{
        // For when the receiver hadn't done a receive()
        // Put it in the msg_to_receive of the receiver's PCB, and note the sender.
        strncpy(dest->msg_to_receive, msg, MSG_MAX);
        //(dest->msg_to_receive)[MSG_MAX] = '\0';
        dest->from_PID = current->PID;
        // Change current running process only when it's not init (i.e. hasn't been blocked)
        if (current != init){
            switch_current_process();
        }
    }
}


// Makes the current process do a Receive. Which means:
// If there is already a message sent to it, receive it (i.e. print it and clear the space.)
// Else, put process into the blocked queue to wait for a "Send".
void Receive(){
    if ((current->msg_to_receive)[0] != '\0'){
        printf("Successfully received message from %d: %s\n", (current->from_PID), (current->msg_to_receive));
        current->from_PID = -1;
        current->msg_to_receive[0] = '\0';
    } else{
        // Block current process
        // (except when process is "init")
        if (current != init){
            List_append(qReceiver, current);
            current->state = BLOCKED;
            printf("No one has sent Process %d a message yet; the process is now blocked waiting for a Send.\n", current->PID);
            // Select the next ready process and set it to running
            switch_current_process();
        } else{
            printf("No one has sent Process %d a message yet; but it is not blocked, since it is \"init\".\n", current->PID);
        }
    }
}


// Delivers reply to sender, and unblocks the sender.
void Reply(int pid, char* msg){
    // Find sender from input PID
    if (pid == current->PID){
        printf("Failure: A process cannot reply to itself. Returning...\n");
        return;
    }
    PCB* sender = NULL;
    if (pid == 0){
        sender = init;
    } else{
        List* list_found = search_process(pid);
        if (list_found == NULL){
            printf("Failure to reply: Process %d not found in system.\n", pid);
            return;
        } else if (list_found != qSender){
            printf("Failure to reply: Process %d has not done a Send.\n", pid);
            return;
        }
        sender = List_curr(list_found);
    }

    // Prepare proc_info for future use (i.e. when the sender is running)
    // Unblock the sender (i.e. put it into ready queue) and raise its priority by 1, if possible.
    sprintf(sender->proc_message, "Received reply from Process %d, message is: %s\n", current->PID, msg);
    //sender->proc_message[PROC_MSG_MAX] = '\0';
    printf("Success: Reply sent.\n");

    if (sender != init){
        printf("Process %d, the original sender, is now unblocked.\n", sender->PID);
        List_remove(qSender);
        sender->state = READY;

        // Put it into the ready queue.
        if (sender->priority == HIGH){
            List_append(qHigh, sender);
        } else if (sender->priority == MID){
            List_append(qHigh, sender);
            sender->priority = HIGH;
            printf("Its priority has been raised to HIGH.\n");
        } else{
            List_append(qMid, sender);
            sender->priority = MID;
            printf("Its priority has been raised to MID.\n");
        }
        if (current == init){
            switch_current_process();
        }
    }
}


// Initializes the named semaphore with the value given.
// ID's can take a value from 0 to 4.
// Can only be done once per semaphore.
void New_sem(int sem_ID, int init_val){
    if (sem_ID < 0 || sem_ID > 4){
        printf("Semaphore creation error - ID can only be from 0 to 4\n");
        return;
    }
    if (semaphores[sem_ID].used == true){
        printf("Semaphore creation error - This semaphore had already been initiated.\n");
        return;
    }
    if (init_val < 0){
        printf("Semaphore creation error - a semaphore's value should be non-negative.\n");
        return;
    }
    semaphores[sem_ID].used = true;
    semaphores[sem_ID].value = init_val;
    printf("Semaphore creation success.\n"
            "Semaphore ID: %d\nSemaphore value: %d\n", sem_ID, init_val);
}


// Executes P operation on named semaphore.
void SemP(int sem_ID){
    if (sem_ID < 0 || sem_ID > 4){
        printf("Input error. Enter a semaphore ID between 0 and 4\n");
        return;
    }
    if (semaphores[sem_ID].used == false){
        printf("Failure to execute P(): This semaphore has not yet been initialized.\n");
        return;
    }
    // Decrease semaphore value. If this drops it below zero, block it (unless it's "init").
    semaphores[sem_ID].value -= 1;
    if (semaphores[sem_ID].value < 0){
        if (current->PID != 0){
            // Block it if it's not init
            List_append(semaphores[sem_ID].qSem, current);
            current->state = BLOCKED;
            printf("Success: Process %d has done a P() operation, and is now blocked on Semaphore %d.\n", current->PID, sem_ID);
        } else{
            // If it's init, then do not block.
            printf("Success: Process %d has done a P() operation on Semaphore %d. It is not blocked because it's init.\n", current->PID, sem_ID);
        }
    } else{
        printf("Success: Process %d has done a P() operation on Semaphore %d.\n", current->PID, sem_ID);
        printf("It is not blocked, because semaphore value is not below 0.\n");
    }
    printf("Current value of this semaphore: %d\n", semaphores[sem_ID].value);
    
    // If current process is blocked, select the next running process
    if (current->state == BLOCKED){
        switch_current_process();
    }
}


// Executes V operation on named semaphore.
void SemV(int sem_ID){
    if (sem_ID < 0 || sem_ID > 4){
        printf("Input error. Enter a semaphore ID between 0 and 4\n");
        return;
    }
    if (semaphores[sem_ID].used == false){
        printf("Failure to execute V(): This semaphore has not yet been initialized.\n");
        return;
    }
    // Increase semaphore value.
    // If value is still <=0, unblock a process from the blocked list of the semaphore.
    semaphores[sem_ID].value += 1;
    if (semaphores[sem_ID].value <= 0){
        List_first(semaphores[sem_ID].qSem);
        PCB* proc = List_remove(semaphores[sem_ID].qSem);
        if (proc == NULL){
            printf("Success. Process %d has done a V() operation on Semaphore %d.\n", current->PID, sem_ID);
            printf("Current value of this semaphore: %d\n", semaphores[sem_ID].value);
            return;
        }
        proc->state = READY;
        printf("Success. Process %d has done a V() operation on Semaphore %d,\n"
                "And has now unblocked Process %d.\n", current->PID, sem_ID, proc->PID);

        // Place process onto ready queue. (if possible, raise its priority by 1)
        // If init is running, make the unblocked process the running process.
        if (proc->priority == HIGH){
            List_append(qHigh, proc);
        } else if (proc->priority == MID){
            List_append(qHigh, proc);
            proc->priority = HIGH;
            printf("Its priority has been raised to HIGH.\n");
        } else{
            List_append(qMid, proc);
            proc->priority = MID;
            printf("Its priority has been raised to MID.\n");
        }
        if (current == init){
            switch_current_process();
        }
        proc = NULL;
    } else{
        printf("Success. Process %d has done a V() operation on Semaphore %d,\n"
                "But no process was unblocked, "
                "because no process had been blocked on this semaphore.\n", current->PID, sem_ID);
    }
    printf("Current value of this semaphore: %d\n", semaphores[sem_ID].value);
}


// Prints all the information of a process (i.e. PCB).
void Procinfo(int pid){
    printf("Here's what we know about Process %d:\n", pid);

    PCB* proc = NULL;
    List* qFound = NULL;
    if (pid == 0){
        proc = init;
        printf("This is the \"init\" process.\n");
        if (current == init){
            printf("It is currently running.\n"
                "This means there are no other processes, or that all other processes are blocked.\n");
        } else{
            printf("It is not running.\n");
            printf("This means that there are other processes in the system, that are not blocked.\n");
        }
    } else if (current->PID == pid){
        printf("Process %d is the currently running process.\n", pid);
        proc = current;
    } else{
        // Find PCB by PID.
        qFound = search_process(pid);
        if (qFound != NULL){
            printf("Process %d exists in the system, but is not currently running.\n", pid);
            proc = List_curr(qFound);
        } else{
            printf("Failure to find process with specified PID in system.\n"
                "Returning...\n");
            proc = NULL;
            qFound = NULL;
            return;
        }
    }
    if (proc != init){
        printf("Process %d is of priority ", proc->PID);
        if (proc->priority == LOW){
            printf("LOW.\n");
        } else if (proc->priority == MID){
            printf("MID.\n");
        } else{
            printf("HIGH.\n");
        }
    }
    printf("Process %d's state is ", proc->PID);
    if (proc->state == RUNNING){
        printf("RUNNING.\n");
    } else if (proc->state == READY){
        printf("READY.\n");
        if (proc != init){
            printf("It is in the respective ready queue of its priority.\n\n");
        }
    } else{
        printf("BLOCKED.\nIt's blocked ");
        if (qFound == qSender){
            printf("because it sent a message and has not yet received a reply.\n\n");
        } else if (qFound == qReceiver){
            printf("because it did a Receive and no one has sent it a message yet.\n\n");
        } else if (qFound == semaphores[0].qSem){
            printf("on Semaphore 0 because it did a P() operation.\n\n");
        } else if (qFound == semaphores[1].qSem){
            printf("on Semaphore 1 because it did a P() operation.\n\n");
        } else if (qFound == semaphores[2].qSem){
            printf("on Semaphore 2 because it did a P() operation.\n\n");
        } else if (qFound == semaphores[3].qSem){
            printf("on Semaphore 3 because it did a P() operation.\n\n");
        } else if (qFound == semaphores[4].qSem){
            printf("on Semaphore 4 because it did a P() operation.\n\n");
        } else{
            printf("... An error occured.\n\n");
            return;
        }
    }  
    if (proc->msg_to_receive[0] != '\0'){
        printf("It has a message to receive, sent by Process %d.\n\n", proc->from_PID);
    }
}


// Displays all process queues and their contents.
void Totalinfo(){
    printf("Displaying all process queues and their contents...\n\n");
    printf("Ready queues ->\n");
    printf("Priority HIGH: ");
    display_queue(qHigh);
    printf("Priority MID: ");
    display_queue(qMid);
    printf("Priority LOW: ");
    display_queue(qLow);
    printf("\nBlocked queues ->\n");
    printf("Processes that has done a SEND: ");
    display_queue(qSender);
    printf("Processes that has done a RECEIVE: ");
    display_queue(qReceiver);
    printf("Processes blocked on Semaphore 0: ");
    display_queue(semaphores[0].qSem);
    printf("Processes blocked on Semaphore 1: ");
    display_queue(semaphores[1].qSem);
    printf("Processes blocked on Semaphore 2: ");
    display_queue(semaphores[2].qSem);
    printf("Processes blocked on Semaphore 3: ");
    display_queue(semaphores[3].qSem);
    printf("Processes blocked on Semaphore 4: ");
    display_queue(semaphores[4].qSem);
}
// Function to print the processes of a given queue, used by Totalinfo().
void display_queue(List* queue){
    if (queue == NULL){
        printf("Failure to display queue - The given queue does not exist.\n");
        return;
    }
    if (List_count(queue) == 0){
        printf("\n");
        return;
    }
    PCB* proc = List_first(queue);
    while (proc != NULL){
        printf("%d  ", proc->PID);
        proc = List_next(queue);
    }
    printf("\n");
}

// Selects a process from the ready queues based on priority,
// and sets it to running.
// If there is no process ready, switch to "init".
// If the newly running process has a proc_msg to print, print it.
void switch_current_process(){
    if (current != NULL){
        printf("Previously running: Process %d\n", current->PID);
    }
    if (List_count(qHigh)){
        List_first(qHigh);
        current = List_remove(qHigh);
        current->state = RUNNING;
    } else if (List_count(qMid)){
        List_first(qMid);
        current = List_remove(qMid);
        current->state = RUNNING;
    } else if (List_count(qLow)){
        List_first(qLow);
        current = List_remove(qLow);
        current->state = RUNNING;
    } else{
        current = init;
        init->state = RUNNING;
    }
    printf("Now running: Process %d\n", current->PID);
    if (current->PID == 0){
        printf("This is the \"init\" process.\n");
        printf("It is running because all the ready queues are empty.\n");
    }
    if (current->proc_message[0] != '\0'){
        printf("%s", current->proc_message);
        current->from_PID = -1;
        current->proc_message[0] = '\0';
    }
}

// Finds process by searching input PID through all queues.
// Returns a pointer to the queue with process as the current item;
// Returns NULL if process not found.
List* search_process(int pid){
    if (current->PID == pid){
        printf("The process searched for is the current process.\n");
        return NULL;
    }
    COMPARATOR_FN compare_PID = &comp_int;
    List_first(qHigh);
    if (List_search(qHigh, compare_PID, &pid)) return qHigh;
    List_first(qMid);
    if (List_search(qMid, compare_PID, &pid)) return qMid;
    List_first(qLow);
    if (List_search(qLow, compare_PID, &pid)) return qLow;
    List_first(qReceiver);
    if (List_search(qReceiver, compare_PID, &pid)) return qReceiver;
    List_first(qSender);
    if (List_search(qSender, compare_PID, &pid)) return qSender;
    List_first(semaphores[0].qSem);
    if (List_search(semaphores[0].qSem, compare_PID, &pid)) return semaphores[0].qSem;
    List_first(semaphores[1].qSem);
    if (List_search(semaphores[1].qSem, compare_PID, &pid)) return semaphores[1].qSem;    
    List_first(semaphores[2].qSem);
    if (List_search(semaphores[2].qSem, compare_PID, &pid)) return semaphores[2].qSem;
    List_first(semaphores[3].qSem);
    if (List_search(semaphores[3].qSem, compare_PID, &pid)) return semaphores[3].qSem;
    List_first(semaphores[4].qSem);
    if (List_search(semaphores[4].qSem, compare_PID, &pid)) return semaphores[4].qSem;
    return NULL;
}
// Function to compare integers, used by search_process().
bool comp_int(void* pItem, void* pComparisonArg){
    //printf("%d vs ", ((PCB*)pItem)->PID);
    //printf("%d\n", *((int*)(pComparisonArg)));
    if (((PCB*)pItem)->PID == *((int*)(pComparisonArg))){
        return true;
    }
    return false;
}

// Free all the ready and blocked queues, as a part of clean up.
void cleanup(){
    FREE_FN free = &free_item;
    List_free(qReceiver, free);
    List_free(qSender, free);
    List_free(qHigh, free);
    List_free(qMid, free);
    List_free(qLow, free);
    List_free(semaphores[0].qSem, free);
    List_free(semaphores[1].qSem, free);
    List_free(semaphores[2].qSem, free);
    List_free(semaphores[3].qSem, free);
    List_free(semaphores[4].qSem, free);
}
// Function required to invoke List_free() in cleanup().
// Does nothing here because the freeing of PCBs is handled by PCB_free().
void free_item(void *pItem){
    // 
}