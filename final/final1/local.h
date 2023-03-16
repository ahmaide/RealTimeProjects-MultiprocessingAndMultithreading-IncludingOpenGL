#define __LOCAL_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <wait.h>
#include <signal.h>
#include <time.h>

struct DUCK{
    char color;
    int location;
    int state;
};

/*union semun {
    int              val;
    struct semid_ds *buf;
    ushort          *array;
};*/

struct MEMORY {
    struct DUCK ducks[125];
};

struct sembuf acquire = {0, -1, SEM_UNDO},
        release = {0,  1, SEM_UNDO};