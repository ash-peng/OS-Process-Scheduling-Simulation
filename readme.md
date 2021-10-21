## Process Scheduling Simulator - Design Decisions
---
    
    The purpose of this file is to document and explain some of the design decisions I made while working for this assignment.

    The specifications noted in the assignment description are closely followed and not further explained. (e.g. This specific simulator uses a preemptive Round Robin scheduling algorithm with three levels of priority; it has 3 ready queues, 2 blocked queues for senders and receivers respectively, and 5 other blocked queues for the 5 semaphores; etc.)

---
### Q: Explain the workings of init.

A: I understand it as the "default" state of the system, and a mechanism to avoid deadlock - where all the processes are blocked and there is nothing left to do. In this particular scenario, init can do Send(), Reply() and V() operations to unblock other processes.

Init can do everything other processes can do; the only difference is that it never gets blocked. In other words, it can do Send(), Receive() and SemP() operations without getting onto their respective blocked queue.

However, as soon as another process is free to run (i.e. is created or unblocked), init hands over control immediately. Unlike other processes, it never waits for the quantum to expire before letting go, since it is the "default" state of the system.

---
### Q: Can process priorities change?

A: In this implementation, yes. This is a design decision to prevent starvation of processes with lower priorities. The mechanism is similar to how UNIX systems generally handle process priorities in multilevel queues.

Specifically, when the quantum expires before a process willingly gives up the CPU, the process's priority gets lowered by 1 (if it is not of the lowest priority already). It is then placed onto the ready queue of its updated priority. This is to potentially decrease the frequency of this process "hogging" the CPU.

On the other hand, when a process is "back" from a blocked queue (i.e. it was previously blocked before the quantum expired), its priority gets leveled up by 1, if possible.

This more flexible way of handling priorities helps to prevent starvation, and gives some lower priority processes a chance of the CPU even when some other processes are favoured at creation time.

---
### Q: Can a process keep a list of incoming messages and Receive() them one at a time?

A: Unfortunately not. A process has at most one message to Receive() at any given time. An old, unreceived message gets overwritten as a newer message is sent to a process.

However, the I command (i.e. Proginfo()) can easily be used to detect whether a given process has a message waiting to be received. With this information, it can then do a Receive() and this clears the incoming buffer.

---
### Q: Can you Kill() a process blocked on a semaphore?

A: Yes, you can. However, something has to be done before the process leaves the system to make up for the semaphore value that was taken away - I increment the semaphore value by 1, before letting it go.

---
### Q: A word on Send(), Receive(), and Reply()?

A: A process cannot Send() to itself: An attempt to Send() to current process would result in failure.

Send() always blocks the sender (if it's not "init"). Receive(), by contrast, only blocks the receiver when no one had sent it a message. If there is indeed a message, Receive() displays the message on screen and clears the buffer.

Any process can Reply() to any other process, as long as the latter exists in the system. This unblocks the replied-to process (if it's not "init") and makes it free to run. When it runs, the reply is displayed on screen.

As previously mentioned, "init" is the exception of blocking in Send() and Receive(): It is never blocked, but apart from that, shares the exact same functionality with other processes.
