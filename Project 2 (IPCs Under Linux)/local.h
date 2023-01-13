#define __LOCAL_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <wait.h>
#include <signal.h>
#include <vector>
#include <time.h>

using namespace std;

struct PERSON {
    long int type;
    pid_t pid;
    char gender;
    char paperType;
    int patience;
    int priority;
    key_t key;
    int hour, min, sec;
};

struct CHANGE {
    long int type;
    pid_t pid;
    int loc;
};

void printPersonInfo(PERSON p) {
    char *temp = (char *) malloc(sizeof(int) * 8);
    sprintf(temp, "%d:%d:%d", p.hour, p.min, p.sec);

    printf("PID: %d\t", p.pid);
    printf("Paper Type: %c\t", p.paperType);
    printf("Gender: %c\t", p.gender);
    printf("Priority: %d\t", p.priority);
    printf("Arrival: %s\n", temp);
    fflush(stdout);
    return;
}

int generate_Randoms(int lower, int upper, pid_t seed) {
    srand(time(NULL) % seed);
    return (int) ((rand() % (upper - lower + 1)) + lower);
}