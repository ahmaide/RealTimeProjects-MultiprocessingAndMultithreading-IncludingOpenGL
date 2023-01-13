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

void handle_sigUSR1(int signum);
void sortPriorityFemales();
void sortPriorityMales();
void allowEntranceFemales();
void allowEntranceMales();
void printData(vector <PERSON> p);
void checkProceses(char gender);
void sendChanges(int pid, char gender);
void sendChanges_2(int pid);

vector <PERSON> males, females;
pid_t metalKey_F, metalKey_M;
bool flag_M = true, flag_F = true;
int msgID, msgID_M, msgID_F;
bool sigTime = false;
int processInfoID;
int UPPER_THRESH, LOWER_THRESH;

int main(int argc, char **argv) {
    key_t entranceKey = atoi(argv[1]);
    metalKey_F = atoi(argv[3]);
    metalKey_M = atoi(argv[2]);
    processInfoID = atoi(argv[4]);
    UPPER_THRESH = atoi(argv[5]);
    LOWER_THRESH = atoi(argv[6]);

    if ((msgID = msgget(entranceKey, IPC_CREAT | 0777)) == -1) {
        perror("Queue for entrace create");
        exit(1);
    }
    if ((msgID_M = msgget(metalKey_M, IPC_CREAT | 0777)) == -1) {
        perror("Queue for males queue create");
        exit(1);
    }
    if ((msgID_F = msgget(metalKey_F, IPC_CREAT | 0777)) == -1) {
        perror("Queue for females queue create");
        exit(1);
    }

    signal(SIGUSR1, handle_sigUSR1);
    while (true) {
        if (males.size() > 0)
            checkProceses('M');
        if (females.size() > 0)
            checkProceses('F');
        PERSON person;
        struct msqid_ds buf;
        msgctl(msgID, IPC_STAT, &buf);
        if (buf.msg_qnum != 0) {
            if ((msgrcv(msgID, &person, sizeof(person), 1, 0)) == -1) {
                perror("reciving at the entrance");
                exit(4);
            }
            if (person.pid == -1) {
                printf("Error with pid = -1\n");
                break;
            }
            if (person.gender == 'M') {
                males.push_back(person);
                sortPriorityMales();
            } 
            else if (person.gender == 'F') {
                females.push_back(person);
                sortPriorityFemales();
            } 
            else {
                printf("gender Error!\n");
                exit(-1000);
            }
        }
        if (sigTime) {
            if (males.size() > 0)
                allowEntranceMales();

            if (females.size() > 0)
                allowEntranceFemales();
            
        }
    }
    exit(1);
}

void sortPriorityMales() {
    sort(males.begin(), males.end(),[](
    const PERSON &a, const PERSON &b){ 
        return (a.priority < b.priority); 
    });
}

void sortPriorityFemales() {
    sort(females.begin(), females.end(),[](
    const PERSON &a,const PERSON &b){ 
        return (a.priority < b.priority); 
    });
}

void checkProceses(char gender){
    if(gender == 'M'){
        for(int i = males.size()-1; i >= 0; i--){
            if(kill(males[i].pid, 0) != 0){
                sendChanges_2(males[i].pid);
                males.erase(males.begin()+i);
            }
        }
    }
    else if (gender == 'F'){
        for(int i = females.size()-1; i >= 0; i--){
            if(kill(females[i].pid, 0) != 0){
                sendChanges_2(females[i].pid);
                females.erase(females.begin()+i);
            }
        }
    }
}

void allowEntranceMales() {
    int i = males.size() - 1;
    struct msqid_ds buf_M;
    msgctl(msgID_M, IPC_STAT, &buf_M);
    if (buf_M.msg_qnum <= LOWER_THRESH) {
        if (msgsnd(msgID_M, &males[i], sizeof(PERSON), 0) == -1) {
            perror("sending from the male queue");
            exit(4);
        } 
        else {
            if(kill(males[i].pid, 0) == 0)
                kill(males[i].pid, SIGUSR1);
            sendChanges(males[i].pid, 'M');
            males.pop_back();
        }
        flag_M = true;
    } 
    else if (buf_M.msg_qnum > LOWER_THRESH && buf_M.msg_qnum < UPPER_THRESH && flag_M) {
        if (msgsnd(msgID_M, &males[i], sizeof(PERSON), 0) == -1) {
            perror("sending from the male queue");
            exit(4);
        } 
        else {
            if(kill(males[i].pid, 0) == 0)
                kill(males[i].pid, SIGUSR1);
            sendChanges(males[i].pid, 'M');
            males.pop_back();
        }
    } 
     if (buf_M.msg_qnum >= UPPER_THRESH) {
        flag_M = false;
    }
    return;
}

void allowEntranceFemales() {
    int i = females.size() - 1;
    struct msqid_ds buf_F;
    msgctl(msgID_F, IPC_STAT, &buf_F);
    if (buf_F.msg_qnum <= LOWER_THRESH) {
        if (msgsnd(msgID_F, &females[i], sizeof(PERSON), 0) == -1) {
            perror("sending from the female queue");
            exit(4);
        } 
        else {
            if(kill(females[i].pid, 0) == 0)
                kill(females[i].pid, SIGUSR1);
            sendChanges(females[i].pid, 'F');
            females.pop_back();
        }
        flag_F = true;
    } 
    else if (buf_F.msg_qnum > LOWER_THRESH && buf_F.msg_qnum < UPPER_THRESH && flag_F) {
        if (msgsnd(msgID_F, &females[i], sizeof(PERSON), 0) == -1) {
            perror("sending from the female queue");
            exit(4);
        } 
        else {
            if(kill(females[i].pid, 0) == 0)
                kill(females[i].pid, SIGUSR1);
            sendChanges(females[i].pid, 'F');
            females.pop_back();
        }
    } 
     if (buf_F.msg_qnum >= UPPER_THRESH) {
        flag_F = false;
    }
    return;
}

void sendChanges(int pid, char gender) {
    CHANGE change;
    change.pid = pid;
    change.type = 1;
    if (gender == 'M')
        change.loc = 1;
    else
        change.loc = 2;
    if (msgsnd(processInfoID, &change, sizeof(CHANGE), 0) == -1) {
        perror("sending changes from entrance");
        exit(4);
    }
}

void sendChanges_2(int pid) {
    CHANGE change;
    change.pid = pid;
    change.type = 1;
    change.loc = 21;
    if (msgsnd(processInfoID, &change, sizeof(CHANGE), 0) == -1) {
        perror("sending changes from entrance");
        exit(4);
    }
}

void handle_sigUSR1(int signum) {
    sigTime = !sigTime;
}