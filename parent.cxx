/*

The PARENT
 */
#include "local.h"

int semid, shmid, croaker;
int semid_empty, semid_full;
char *shmptr;
int consumerCount = 50;
int producerCount = 3;
int signalCount = 0;

pid_t p_id[3];
pid_t c_id[50];
pid_t pid = getpid();

//signal handler to catch SIGUSR1 signal
//consumer processes signals the parent when they are done
//if all 50 of the consumer processes are done
//then clean shared memory and remove semaphores
//as well as kill all child processes.
void sigUsrHandler(int signum)
{
    signalCount++;
    if (signalCount >= consumerCount)
    {
        int i;
        for (i = 0; i < consumerCount; i++)
        {
            kill(c_id[i], SIGKILL);
        }

        int j;
        for (j = 0; j < producerCount; j++)
        {
            kill(p_id[j], SIGKILL);
        }

        shmdt(shmptr);

        shmctl(shmid, IPC_RMID, (struct shmid_ds *)0);

        semctl(semid, 0, IPC_RMID, 0);

        semctl(semid_empty, 0, IPC_RMID, 0);

        semctl(semid_full, 0, IPC_RMID, 0);

        exit(0);
    }
}

int main(int argc, char *argv[])
{

    static struct MEMORY memory;

    static unsigned int short start_val[2] = {1, 0};

    //create a signal handler to catch consumer processes
    //when they finish their consume event.
    signal(SIGUSR1, sigUsrHandler);

    union semun arg;
    memory.head = memory.tail = 0;

    srand(time(NULL) ^ getpid());

    // Create, attach, clear segment

    if ((shmid = shmget((int)pid, sizeof(memory), IPC_CREAT | 0600)) != -1)
    {
        if ((shmptr = (char *)shmat(shmid, 0, 0)) == (char *)-1)
        {
            perror("shmptr -- parent -- attach ");
            return 2;
        }

        memcpy(shmptr, (char *)&memory, sizeof(memory));
    }
    else
    {
        perror("shmid -- parent -- creation ");

        return 3;
    }

    // Create & initialize semaphores

    if ((semid = semget((int)pid, 2, IPC_CREAT | 0666)) != -1)
    {

        arg.array = start_val;

        if (semctl(semid, 0, SETALL, arg) == -1)
        {

            perror("semctl -- parent -- initialization");

            return 4;
        }
    }
    else
    {

        perror("semget -- parent -- creation ");

        return 5;
    }

    //create a second semaphore to syncronize empty
    if ((semid_empty = semget((int)pid + 1, 1, IPC_CREAT | 0666)) != -1)
    {

        if (semctl(semid_empty, 0, SETVAL, 15) == -1)
        {

            perror("semctl -- parent -- initialization");

            return 4;
        }
    }
    else
    {

        perror("semget -- parent -- creation ");

        return 5;
    }

    //create a second semaphore to syncronize full
    if ((semid_full = semget((int)pid + 2, 1, IPC_CREAT | 0666)) != -1)
    {

        if (semctl(semid_full, 0, SETVAL, 0) == -1)
        {

            perror("semctl -- parent -- initialization");

            return 4;
        }
    }
    else
    {

        perror("semget -- parent -- creation ");

        return 5;
    }

    // Fork PRODUCER process
    int i;
    for (i = 0; i < producerCount; i++)
    {
        if ((p_id[i] = fork()) == -1)
        {
            perror("fork -- producer");
            return 6;
        }
        else if (p_id[i] == 0)
        {
            execl("producer", "./producer", "", (char *)0);
            perror("execl -- producer");
            return 7;
        }
    }

    // Fork CONSUMER process
    int j;
    for (j = 0; j < consumerCount; j++)
    {
        int randomNumber = rand() % 5 + 1;
        sleep(randomNumber);
        if ((c_id[j] = fork()) == -1)
        {

            perror("fork -- consumer");

            return 8;
        }
        else if (c_id[j] == 0)
        {

            execl("consumer", "./consumer", "", (char *)0);

            perror("execl -- consumer");

            return 9;
        }
    }

    // Infinite pause untill sigUsr1 signal event terminates
    //the whole application
    while (1)
    {
        pause();
    }

    return 0;
}
