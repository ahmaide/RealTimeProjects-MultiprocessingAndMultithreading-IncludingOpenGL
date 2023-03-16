#include "local.h"

void createPlayers(int memoryID);

int playersIDs[2];
int messageQueuesIDs[2];
void setDucksLocations(int shmid);
void startRound();
void randomlyChoose10Ducks();

#define EXCEED =60;
# define MISSED = 20;

int curr2=0;
int curr1=0;

int main() {
    static struct  MEMORY memory;
    int            shmid;
    char          *shmptr;
    union semun    arg;

    if ( (shmid = shmget((int) pid, sizeof(memory), IPC_CREAT | 0600)) != -1 ) {
        if ( (shmptr = (char *) shmat(shmid, 0, 0)) == (char *) -1 ) {
            perror("shmptr -- parent -- attach");
            exit(1);
        }
        memcpy(shmptr, (char *) &memory, sizeof(memory));
    }
    else {
        perror("shmid -- parent -- creation");
        exit(2);
    }

    createPlayers(shmid);
    sleep(1);
    void setDucksLocations(int shmid);
    while(curr1<EXCEED && curr2<MISSED){
        startRound();
        sleep(1);
        // endRoundFunction
    }

    kill(SIGKILL, playersIDs[0]);
    kill(SIGKILL, playersIDs[1]);
    return 0;
}

void createPlayers(int memoryID){
    char id = 'A';
    for(int i=0; i<2 ; i++){
        key_t k;
        if ((k = ftok(".", id)) == -1) {
            perror("message queue keys generation");
            exit(1);
        }
        if ((k = ftok(".", id)) == -1) {
            perror("message queue keys generation");
            exit(1);
        }
        if ((messageQueuesIDs[i] = msgget(k, IPC_CREAT | 0777)) == -1) {
            perror("Queue create for metal detector");
            exit(1);
        }
        playersIDs[i] = fork();
        char *messageQueueID = (char *) malloc(sizeof(int));
        sprintf(messageQueueID, "%d", messageQueuesIDs[i]);
        char *memID = (char *) malloc(sizeof(int));
        sprintf(memID, "%d", memoryID);
        if (playersIDs[i] != -1) {
            if (playersIDs[i] == 0) {
                const char *programname = "./PLAYER";
                execl(programname, programname, messageQueueID, memID, NULL);
            }
        }
        else {
            perror("fork while creating office");
            exit(1);
        }
    }
}

void setDucksLocations(int shmid){
    // here the ducks are randomly distributed by color in the memory
}

void startRound(){
    randomlyChoose10Ducks();
    for(int i=0; i<2; i++)
        kill(playersIDs[i], SIGINT);
}

void randomlyChoose10Ducks(){
    // here the 10 ducks are randomly chosen
}