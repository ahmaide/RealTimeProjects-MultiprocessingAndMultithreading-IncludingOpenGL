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
#include "local.h"
#include <unistd.h>
using namespace std;

void send_to_office();
void send_Changes_to_parent(int pid, char gender);
void sendChanges_2(int pid, char gender);
bool checkProceses(int pid, char gender);

int msgID, msgID_O;
pid_t metalKey_o;
int processInfoID, METAL_LOWER, METAL_UPPER;
char metDetType;

int main(int argc, char **argv) {
    key_t metalKey = atoi(argv[1]);
    metDetType = argv[2][0];
    metalKey_o = atoi(argv[3]);
    processInfoID = atoi(argv[4]);
    METAL_LOWER = atoi(argv[5]);
    METAL_UPPER = atoi(argv[6]);

    if ((msgID = msgget(metalKey, IPC_CREAT | 0777)) == -1) {
        perror("Queue create for metal detector");
        exit(1);
    }
    if ((msgID_O = msgget(metalKey_o, IPC_CREAT | 0777)) == -1) {
        perror("Queue create for metal detector changes");
        exit(1);
    }
    
    while (true) {
        PERSON person;
        struct msqid_ds buf;
        msgctl(msgID, IPC_STAT, &buf);

        if (buf.msg_qnum != 0) {
            if ((msgrcv(msgID, &person, sizeof(PERSON), 1, 0)) == -1) {
                perror("reciving at the metal detector");
                exit(4);
            }
            send_Changes_to_parent(person.pid, person.gender);
            if (person.gender == 'M') {
                sleep(generate_Randoms(METAL_LOWER, METAL_UPPER, getpid()));
            } 
            else if (person.gender == 'F') {
                sleep(generate_Randoms(METAL_LOWER*2, METAL_UPPER*2, getpid()));
            } 
            else {
                printf("gender Error!\n");
                exit(-1);
            }
            if(checkProceses(person.pid, person.gender)){
                if (msgsnd(msgID_O, &person, sizeof(person), 0) == -1) {
                    perror("sending to waiting room");
                    exit(4);
                }   
            }
        }    
    }
    exit(1);
}

void send_Changes_to_parent(int pid, char gender) {
    CHANGE change;
    change.pid = pid;
    if (gender == 'M')
        change.loc = 3;
    else
        change.loc = 4;
    change.type = 1;
    if (msgsnd(processInfoID, &change, sizeof(change), 0) == -1) {
        perror("sending metal detector changes");
        exit(4);
    }
}

void sendChanges_2(int pid, char gender) {
    CHANGE change;
    change.pid = pid;
    if(gender == 'M')
        change.loc = 23;
    else
        change.loc = 24;
    change.type = 1;
    if (msgsnd(processInfoID, &change, sizeof(CHANGE), 0) == -1) {
        perror("sending changes from entrance");
        exit(4);
    }
}

bool checkProceses(int pid, char gender){
    if(kill(pid, 0) != 0){
        sendChanges_2(pid, gender);
        return false;
    }
    return true; 
}