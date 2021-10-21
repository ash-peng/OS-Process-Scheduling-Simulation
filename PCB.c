#include "PCB.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int  PID_pool = 0;

// Create a new process (i.e. PCB) with the given priority.
// Returns pointer to the new PCB if success; NULL if failure.
PCB* PCB_create(int priority){
    PCB* new_PCB = malloc(sizeof(PCB));
    if (new_PCB == NULL){
        return NULL;
    }
    new_PCB->PID = PID_pool;
    PID_pool++;
    new_PCB->state = READY;
    new_PCB->priority = priority;
    memset(new_PCB->msg_to_receive, '\0', MSG_MAX);
    new_PCB->from_PID = -1;
    memset(new_PCB->proc_message, '\0', PROC_MSG_MAX);
    return new_PCB;
}

// Frees given process (i.e. PCB).
void PCB_free(PCB* proc_to_del){
    if (proc_to_del == NULL){
        printf("Failure to delete PCB - pointer points to NULL.\n");
        return;
    }
    free(proc_to_del);
    proc_to_del = NULL; 
}

