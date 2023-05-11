#include "product.h"

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

union semun
{
    int val;               /* Value for SETVAL */
    struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* Array for GETALL, SETALL */
    struct seminfo *__buf; /* Buffer for IPC_INFO
                              (Linux-specific) */
};

int main()
{
    int key = ftok(".", 'a');
    int semid = semget(key, MAX_PRODUCTS + 1, IPC_CREAT | 0666);
    union semun su;
    unsigned short arr[MAX_PRODUCTS + 1];
    for (int i = 0; i < MAX_PRODUCTS; i++)
    {
        arr[i] = 1;
    }
    arr[MAX_PRODUCTS] = 5;
    su.array = arr;
    semctl(semid, 0, SETALL, su);
    printf("Semaphore created\n");
    int fd = open(DATAFILE, O_RDONLY, 0);
    if (fd < 0)
    {
        create_datafile();
        printf("Datafile created\n");
        fd = open(DATAFILE, O_RDONLY, 0);
    }
    close(fd);
    return 0;
}