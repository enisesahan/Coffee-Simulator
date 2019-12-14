/*

The PRODUCER ...
 */
#include "local.h"
int main(int argc, char *argv[])
{

    int r, c, semid, semid_empty, semid_full, shmid;

    pid_t ppid = getppid();

    char *shmptr;

    struct MEMORY *memptr;

    // Access, attach & ref mem

    if ((shmid = shmget((int)ppid, 0, 0)) != -1)
    {

        if ((shmptr = (char *)shmat(shmid, (char *)0, 0)) == (char *)-1)
        {

            perror("shmat -- producer -- attach ");

            return 21;
        }

        memptr = (struct MEMORY *)shmptr;
    }
    else
    {

        perror("shmget -- producer -- access ");

        return 22;
    }

    // Access semaphore set

    if ((semid = semget((int)ppid, 2, 0)) == -1)
    {

        perror("semget -- producer -- access ");

        return 23;
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

    acquire.sem_num = AVAIL_SLOTS; //0
    release.sem_num = TO_CONSUME;  //1

    acquire_empty.sem_num = AVAIL_SLOTS;
    release_empty.sem_num = AVAIL_SLOTS;

    acquire_full.sem_num = AVAIL_SLOTS;
    release_full.sem_num = AVAIL_SLOTS;

    while (1)
    {
        //acquire the first semaphore in the binary semaphore's set
        //and try to access the buffer.
        if (semop(semid, &acquire, 1) == -1)
        {

            perror("semop -- producer -- acquire ");

            return 24;
        }

        //create a random number between 1 and 4
        c = (rand() % 4) + 1;

        for (r = 0; r < c; r++)
        {
            int empty_val = semctl(semid_empty, 0, GETVAL, 0);

            //if the empty semaphore is 0 break from the loop
            //and give the turn to consumer processes.
            //this means there is no space to write and buffer is full.
            if (empty_val == 0)
            {
                break;
            }
            //get the empty semaphore and access writing critical section.
            if (semop(semid_empty, &acquire_empty, 1) == -1)
            {

                perror("semop -- producer -- acquire ");

                return 24;
            }

            //critical section start

            cout << "Barrista [ " << getpid() << " ] prepared a coffee!" << endl;
            strcpy(memptr->buffer[memptr->tail], "f");
            memptr->tail = (memptr->tail + 1) % N_SLOTS;

            //critical section end

            //release and increment the full semaphore by one.
            //by doing this mark the semaphore as there is something to read.
            if (semop(semid_full, &release_full, 1) == -1)
            {
                perror("semop -- producer -- release ");
                return 25;
            }
        }

        //release the second semaphore in the binary semaphore's set
        //and give the turn to a consumer process.
        if (semop(semid, &release, 1) == -1)
        {

            perror("semop -- producer -- release ");

            return 25;
        }

        sleep(60);
    }

    return 0;
}
