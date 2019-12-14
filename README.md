# Coffee-Simulator
Producer-Consumer Problem using Semaphores & Shared Memory
- Shared Memory uses 15 slots for coffe production, 1 int count Head and 1
int count Tail.<br>
- Semaphores are used. One binary semaphore controls
accessing to 15 slots, one counting semaphore is for controlling number of empty slots and
one counting semaphore for controlling number of full slots.<br>
- 3 Baristas (These are producer processes), 50 Clients (These are consumer processes).<br>
- Producer and Consumer processes use semaphore to wake each other.

