#include <iostream>
#include <GL/glut.h>
#include <stdlib.h>
#include <vector>
#include <cstring>
#include <fstream>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <bits/stdc++.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "local.h"
#include <unistd.h>
using namespace std;

#ifdef __linux__
union semun {
    int              val;
    struct semid_ds *buf;
    ushort          *array;
};

#endif

int msgID;
int processInfoID;
char officeType;
int satisfied;

void send_Changes_to_parent(int pid);

void send_Changes_to_parent2(int pid);

int main(int argc, char **argv) {
    key_t officeKey = atoi(argv[1]);
    officeType = argv[2][0];
    key_t semKey = atoi(argv[3]);
    processInfoID = atoi(argv[4]);
    int order = atoi(argv[5]);
    int OFFICE_UPPER = atoi(argv[6]);
    int OFFICE_LOWER = atoi(argv[7]);

    static struct sembuf acquire = {0, -1, SEM_UNDO}, release = {0, 1, SEM_UNDO};
    static ushort start_val[1] = {1};
    union semun arg;
    int semid;
    if (order != 0)
        usleep(500);

    if ((msgID = msgget(officeKey, IPC_CREAT | 0777)) == -1) {
        perror("Queue create for office");
        exit(1);
    }
    if ((semid = semget(semKey, 1, IPC_CREAT | 0777)) == -1) {
        perror("semaphore create for office");
        exit(1);
    }
    if (order == 0) {
        arg.array = start_val;
        if (semctl(semid, 0, SETALL, arg) == -1) {
            perror("first office initialization");
            exit(1);
        }
    }
    while (true) {
        acquire.sem_num = 0;
        if (semop(semid, &acquire, 1) == -1) {
            perror("waiting as it's not the turn to read for window");
            exit(3);
        }
        PERSON person;
        struct msqid_ds buff;
        msgctl(msgID, IPC_STAT, &buff);
        if (buff.msg_qnum != 0) {
            if ((msgrcv(msgID, &person, sizeof(person), 1, 0)) == -1) {
                perror("receiving at the office");
                exit(4);
            }
            //cout << "office with id " << getpid() << " for " << officeType << " got " << person.pid << endl;
            release.sem_num = 0;
            if (semop(semid, &release, 1) == -1) {

                perror("release semaphore");
                exit(5);
            }
            send_Changes_to_parent(person.pid);
            if(kill(person.pid, 0) == 0){
                usleep(500000);
                kill(person.pid, SIGUSR2);
            }
            sleep(generate_Randoms(OFFICE_LOWER, OFFICE_UPPER, getpid()));
            //cout << person.pid << " has pass the office type (" << officeType << ")now" << endl;
            satisfied = generate_Randoms(0, 10, getpid());
            send_Changes_to_parent2(person.pid);
        } else {
            release.sem_num = 0;
            if (semop(semid, &release, 1) == -1) {
                perror("release semaphore");
                exit(5);
            }
        }

    }
    exit(1);
}

void send_Changes_to_parent(int pid) {
    CHANGE change;
    change.pid = pid;
    if (officeType == 'B') {
        change.loc = 7;
    } else if (officeType == 'T') {
        change.loc = 8;
    } else if (officeType == 'R') {
        change.loc = 9;
    } else {
        change.loc = 10;
    }
    change.type = 1;
    if (msgsnd(processInfoID, &change, sizeof(change), 0) == -1) {
        perror("Producer msgsend");
        exit(4);
    }
}

void send_Changes_to_parent2(int pid) {
    int add =0;
    if(satisfied>2)
        add =4;
    CHANGE change;
    change.pid = pid;
    if (officeType == 'B') {
        change.loc = 11+add;
    } else if (officeType == 'T') {
        change.loc = 12+add;
    } else if (officeType == 'R') {
        change.loc = 13+add;
    } else {
        change.loc = 14+add;
    }
    change.type = 1;
    if (msgsnd(processInfoID, &change, sizeof(change), 0) == -1) {
        perror("Producer msgsend");
        exit(4);
    }
}