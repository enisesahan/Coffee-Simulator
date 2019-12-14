/*

The CONSUMER

*/
#include "local.h"
int main(int argc, char *argv[])
{

    static char local_buffer[SLOT_LEN];

    int i, r, c, semid, semid_empty, semid_full, shmid;

    pid_t ppid = getppid();

    char *shmptr;

    struct MEMORY *memptr;

    // Access, attach & ref memory
    if ((shmid = shmget((int)ppid, 0, 0)) != -1)
    {
        if ((shmptr = (char *)shmat(shmid, (char *)0, 0)) == (char *)-1)
        {
            perror("shmat -- consumer -- attach");
            return 31;
        }
        memptr = (struct MEMORY *)shmptr;
    }
    else
    {
        perror("shmget -- consumer -- access");
        return 32;
    }

    // Access semaphore set
    if ((semid = semget((int)ppid, 2, 0)) == -1)
    {
        perror("semget -- consumer -- access ");
        return 33;
    }

    if ((semid_empty = semget((int)ppid + 1, 1, 0)) == -1)
    {

        perror("semget -- producer -- access ");

        return 23;
    }

    if ((semid_full = semget((int)ppid + 2, 1, 0)) == -1)
    {

        perror("semget -- producer -- access ");

        return 23;
    }

    //set the random seed based on time and pid
    srand(time(NULL) ^ getpid());

    acquire.sem_num = TO_CONSUME;
    release.sem_num = AVAIL_SLOTS;

    acquire_empty.sem_num = AVAIL_SLOTS;
    release_empty.sem_num = AVAIL_SLOTS;

    acquire_full.sem_num = AVAIL_SLOTS;
    release_full.sem_num = AVAIL_SLOTS;

    cout << "A new costumer has arrived!" << endl;

    //acquire the second semaphore in the binary semaphore's set
    //and try to access the buffer.
    if (semop(semid, &acquire, 1) == -1)
    {
        perror("semop -- consumer -- acquire ");
        return 34;
    }

    //generate a random number either 1 or 2.
    c = (rand() % 2) + 1;

    for (r = 0; r < c; r++)
    {
        int full_val = semctl(semid_full, 0, GETVAL, 0);

        //if the full semaphore is 15 break from the loop
        //and give the turn to consumer processes.
        //this means there is nothing to read.
        if (full_val == 0)
        {
            break;
        }
        if (semop(semid_full, &acquire_full, 1) == -1)
        {

            perror("semop -- producer -- acquire ");

            return 24;
        }

        //critical section start
        cout << "Costumer [ " << getpid() << " ] consumed a coffee!" << endl;
        memset(local_buffer, '\0', sizeof(local_buffer));
        strcpy(local_buffer, memptr->buffer[memptr->head]);
        memptr->head = (memptr->head + 1) % N_SLOTS;

        //critical section end

        //release and increment the empty semaphore by one.
        //by doing this mark the semaphore as there is a space to write.
        if (semop(semid_empty, &release_empty, 1) == -1)
        {

            perror("semop -- producer -- release ");

            return 25;
        }
    }

    //release the first semaphore in the binary semaphore's set
    //and give the turn to a producer process.
    if (semop(semid, &release, 1) == -1)
    {
        perror("semop -- consumer -- release ");
        return 35;
    }

    //signal parent process when consuming is done
    kill(getppid(), SIGUSR1);

    //wait for parent to kill
    pause();

    return 0;
}
