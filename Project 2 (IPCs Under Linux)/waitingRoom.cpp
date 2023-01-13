#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <signal.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <bits/stdc++.h>
#include <map>
#include "local.h"
#include <unistd.h>
using namespace std;

void send_Changes_to_parent(int pid, char gender);
void checkProcesses();
void checkProcesses_2();
void sendChanges_2(int pid, char gender);

int msgID;
int msgID_O;
int numOfChairs;
map<pid_t, PERSON> people;
map<pid_t, PERSON> peopleByPaper[4];
int processInfoID;
int messagesID[4];
int availableWindows[4];

int main(int argc, char **argv) {
    key_t metalKey = atoi(argv[1]);
    numOfChairs = atoi(argv[2]);
    key_t officesKeys[4];
    for(int i=0; i<4; i++) 
        availableWindows[i] = atoi(argv[i+8]);
    
    for (int i = 0; i < 4; i++)
        officesKeys[i] = atoi(argv[i + 3]);

    processInfoID = atoi(argv[7]);
    if ((msgID = msgget(metalKey, IPC_CREAT | 0777)) == -1) {
        perror("Queue create for waiting room");
        exit(1);
    }
    for (int i = 0; i < 4; i++) {
        if ((messagesID[i] = msgget(officesKeys[i], IPC_CREAT | 0777)) == -1) {
            perror("Queue create for waiting room changes");
            exit(1);
        }
    }
    while (true) {
        if(people.size() > 0)
            checkProcesses();
        if(!peopleByPaper->empty())
            checkProcesses_2();
        PERSON person;
        struct msqid_ds buf;
        msgctl(msgID, IPC_STAT, &buf);
        if (buf.msg_qnum != 0) {
            if ((msgrcv(msgID, &person, sizeof(person), 1, 0)) == -1) {
                perror("waiting room receiving");
                exit(4);
            }
            people[person.pid] = person;
            switch (person.paperType) {
                case 'B':
                    peopleByPaper[0][person.pid] = person;
                    break;
                case 'T':
                    peopleByPaper[1][person.pid] = person;
                    break;
                case 'R':
                    peopleByPaper[2][person.pid] = person;
                    break;
                default:
                    peopleByPaper[3][person.pid] = person;
                    break;
            }
            send_Changes_to_parent(person.pid, person.gender);
        }
        for (int i = 0; i < 4; i++) {
            struct msqid_ds buf_2;
            msgctl(messagesID[i], IPC_STAT, &buf_2);

            while(buf_2.msg_qnum<1 && peopleByPaper[i].size()!=0) {
                PERSON person2;
                person2 = peopleByPaper[i].begin()->second;
                if ((kill(person2.pid, 0) == 0) && person2.pid > 0) {
                    if (msgsnd(messagesID[i], &person2, sizeof(PERSON), 0) == -1) {
                        perror("sending to office from waiting room");
                    }
                }
                peopleByPaper[i].erase(person2.pid);
                people.erase(person2.pid);
            }
            for(int j=1; j<4 && buf_2.msg_qnum<1; j++) {
                struct msqid_ds buf_3;
                msgctl(messagesID[(i + j) % 4], IPC_STAT, &buf_3);
                if (buf_3.msg_qnum != 0 && buf_2.msg_qnum < 1 && peopleByPaper[(i + j) % 4].size() != 0) {
                    PERSON person2;
                    person2 = peopleByPaper[(i + j) % 4].begin()->second;
                    if ((kill(person2.pid, 0) == 0) && person2.pid > 0) {
                        if (msgsnd(messagesID[(i + j) % 4], &person2, sizeof(PERSON), 0) == -1) {
                            perror("sending to office from waiting room");
                        }
                    }
                    peopleByPaper[(i + j) % 4].erase(person2.pid);
                    people.erase(person2.pid);
                }
            }
        }
    }
    return 0;
}

void sendChanges_2(int pid, char gender) {
    CHANGE change;
    change.pid = pid;
    if (gender == 'M')
        change.loc = 25;
    else
        change.loc = 26;
    change.type = 1;
    if (msgsnd(processInfoID, &change, sizeof(CHANGE), 0) == -1) {
        perror("sending changes from entrance");
        exit(4);
    }
}

void checkProcesses(){
    for(int i = people.size()-1; i >= 0; i--){
        if(kill(people[i].pid, 0) != 0){
            sendChanges_2(people[i].pid, people[i].gender);
            people.erase(i);
        }
    }
}

void checkProcesses_2(){
    map<pid_t, PERSON>::iterator it;
    for(int i = 0; i<4; i++){
        for (it = peopleByPaper[i].begin(); it != peopleByPaper[i].end(); it++){
            if(kill(it->first, 0) != 0){
                sendChanges_2(it->first, peopleByPaper[i][it->first].gender);
                peopleByPaper[i].erase(it->first);
            }
        }
    }
}

void send_Changes_to_parent(int pid, char gender) {
    CHANGE change;
    change.pid = pid;
    if (gender == 'M')
        change.loc = 5;
    else
        change.loc = 6;
    change.type = 1;
    if (msgsnd(processInfoID, &change, sizeof(change), 0) == -1) {
        perror("sending waiting room changes");
        exit(4);
    }
}