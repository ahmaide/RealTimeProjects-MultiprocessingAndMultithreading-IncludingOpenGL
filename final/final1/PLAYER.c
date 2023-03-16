#include "local.h"
#include <cstring>

void signal_catcher(int);
void signal_catcher2(int);

double attentionRate;
int shmID;
int MessageQID;
int points =0;

int fishing = 0;

int main(int argc, char **argv) {

    MessageQID = stoi(argv[1]);
    shmID = stoi(argv[2]);

    if ( sigset(SIGINT, signal_catcher) == SIG_ERR ) {
        perror("Sigset can not set SIGINT");
        exit(SIGINT);
    }

    return 0;
}

void signal_catcher(int p){
    fishing =1;
    while(fishing ==1){
        // start fishing
    }
}

void signal_catcher2(int p){

    fishing = 0;
    attentionRate*0.92;
}
