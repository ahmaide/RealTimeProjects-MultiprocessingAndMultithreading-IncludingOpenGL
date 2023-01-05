#include <iostream>
#include <GL/glut.h>
#include <vector>
#include <cstring>
#include <stdlib.h>
#include <fstream>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define UPPER_TIME_LIMIT 8
#define LOWER_TIME_LIMIT 3
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

using namespace std;

void signal_catcher(int);
void signal_catcher2(int);
int generateSpeed();

int n;
string team;
int pos;
double tiredness = 1;
int initialPoint;
int finalPoint;
int speed;
char *fifo;

int main(int argc, char **argv) {
    pos = stoi(argv[1]);

    if(stoi(argv[2]) == 1){
        team = "red";
    }
    else{
        team = "green";
    }
    
    n = stoi(argv[3]);
    initialPoint = stoi(argv[4]);
    finalPoint = stoi(argv[5]);
    fifo = argv[6];

    if(mkfifo(fifo, 0777) == -1){
        if(errno != EEXIST){
            cout << "Could not create fifo file" << endl;
            exit(-1);
        }
    }

    speed = generateSpeed();
    int fd = open(fifo, O_WRONLY);

    if(team == "green")
        cout << ANSI_COLOR_GREEN << getpid() << " from " << team << " team, with speed: " << speed << ANSI_COLOR_RESET << endl;
    else cout << ANSI_COLOR_RED << getpid() << " from " << team << " team, with speed: " << speed << ANSI_COLOR_RESET << endl;
    
    if(write(fd, &speed, sizeof(int)) == -1){
        cout << "File is not found :(" << endl;
        exit(-1);
    }

    close(fd);

    if ( sigset(SIGINT, signal_catcher) == SIG_ERR ) {
        perror("Sigset can not set SIGINT");
        exit(SIGINT);
    }

    while(1);

    return 0;
}

void signal_catcher(int p){
    tiredness = tiredness * 0.95;
    speed = generateSpeed()*tiredness;
    int fd = open(fifo, O_WRONLY);
    
    if(team == "green")
        cout << ANSI_COLOR_GREEN << getpid() << " from " << team << " team, with speed: " << speed << ANSI_COLOR_RESET << endl;
    else cout << ANSI_COLOR_RED << getpid() << " from " << team << " team, with speed: " << speed << ANSI_COLOR_RESET << endl;
    
    if(write(fd, &speed, sizeof(int)) == -1){
        cout << "File is not found :(" << endl;
        exit(-1);
    }

    close(fd);
}

int generateSpeed(){
    srand(time(NULL)%getpid());
    int speed = rand()%UPPER_TIME_LIMIT + LOWER_TIME_LIMIT;
    return speed; 
}

