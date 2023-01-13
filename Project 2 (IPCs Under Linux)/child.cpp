#include <iostream>
#include <stdlib.h>
#include <signal.h>
#include "local.h"
#include <unistd.h>
using namespace std;

void handle_SIGUSR1(int signum);
void handle_SIGUSR2(int signum);
void handle_SIGTERM(int signum);

int patienceFreq, patience;

int main(int argc, char** argv){
    signal(SIGUSR1, handle_SIGUSR1);
    signal(SIGUSR2, handle_SIGUSR2);
    signal(SIGTERM, handle_SIGTERM);

    patience = atoi(argv[1]);
    patienceFreq = atoi(argv[2]);
    pid_t myPID = getpid();

    while(true){
        sleep(patienceFreq);
        patience--;
        if(patience == 0)
            break;
    }
    cout << myPID << ": running out of patience and will leave" << endl;
    exit(0);
}

void handle_SIGUSR1(int signum){
    patience+=5;
    return;
}

void handle_SIGUSR2(int signum){
    patience+=20;
    return;
}

void handle_SIGTERM(int signum){
    exit(0);
}
