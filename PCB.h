// PCB Data Type

#ifndef _PCB_H_
#define _PCB_H_
#include <stdbool.h>

#define MSG_MAX 40
#define PROC_MSG_MAX 200

enum proc_state{
    READY,
    RUNNING,
    BLOCKED
};

enum proc_priority{
    HIGH,
    MID,
    LOW
};

struct PCB{
	int PID;
	int state;
    int priority;
    char msg_to_receive[MSG_MAX];
    int from_PID;
    char proc_message[PROC_MSG_MAX];
};
typedef struct PCB PCB;

// Create a new process (i.e. PCB) with the given priority.
// Returns pointer to the new PCB if success; NULL if failure.
PCB* PCB_create(int priority);

// Frees given process (i.e. PCB).
void PCB_free(PCB* proc_to_del);

#endif