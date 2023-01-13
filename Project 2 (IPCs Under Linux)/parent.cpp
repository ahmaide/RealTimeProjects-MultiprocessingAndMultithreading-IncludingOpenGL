#include <iostream>
#include <GL/glut.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <map>
#include <time.h>
#include "local.h"
#include <bits/stdc++.h>
#include <unistd.h>
using namespace std;

void checkEnd();
void handle_SIGCHID(int sig, siginfo_t *info, void *context);
void InitializeKeys();
void getPersonInfoID();
void display();
void startOpenGL();
void reshape(int w, int h);
void timer(int);
void renderBitMap(double x, double y, void *font, char *string);
void displayTime();
void createNewPerson();
void displayItems();
void displayEntrance();
void displayQueue();
void displayStandingUp();
void displayStatistics();
void initializePlaces();
void initializeEntrance(char *temp4);
void initializeMetalDet(char *temp4, char *tempNextKey);
void initializeWaitingRoom(char *temp4, char *tempNextKey);
void initializeOffices(char *temp4, char *officeKey, char *type, char *semaKey, int ord);
void displayOfficesNames();
PERSON defineNewPerson(pid_t pid);
void checkNewLocations();
void removeAllQueuesSem();
void handle_SIGINT(int signum);
void readFile();
void generateNumberOfTeller();

int h = 5;
int m = 0;
int s = 0;
int seed = 0;
int minRate, secRate;
int totalNumOfPeople = 0;
vector<PERSON> allPeople;
map<pid_t, PERSON> people[10];
int entranceID;
vector <pid_t> placesID;
vector <key_t> placesKeys;
key_t semKeys[4];
int msgID;
int INITIAL_CHAIR = 100;
int processInfoID, numberOfKeys = 11;
int metalMaleLight = 0;
int metalFemaleLight = 0;
char offices[4] = {'B', 'T', 'R', 'I'};
int totalNumOfTellers;
int options[4];
int numOfWindows[4] = {0, 0, 0, 0};
bool termFlag = false;
int MAX_NUMBER_OF_PEOPLE, UPPER_QUEUE_THRESH, 
    LOWER_QUEUE_THRESH, PATIENCE_RANGE_LOWER, PATIENCE_RANGE_UPPER;
int PATIENCE_FREQ, METAL_LOWER, METAL_UPPER, 
    ARRIVAL_RATE, OFFICE_UPPER, OFFICE_LOWER;
int UNSERVED = 0, UNHAPPY = 0, SATISFIED = 0;
int goalUNSERVED = 10, goalUNHAPPY = 10, goalSATISFIED = 10;

int main(int argc, char **argv) {
    signal(SIGINT, handle_SIGINT);
    removeAllQueuesSem();
    readFile();
    generateNumberOfTeller();
    InitializeKeys();
    getPersonInfoID();
    initializePlaces();

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handle_SIGCHID;
    sigaction(SIGCHLD, &sa, NULL);

    if ((msgID = msgget(placesKeys[1], IPC_CREAT | 0777)) == -1) {
        perror("Key for entrance doesn't exist");
        exit(1);
    }

    glutInit(&argc, argv);
    startOpenGL();
    return 0;
}

void InitializeKeys() {
    char id = 'A';
    for (int i = 0; i < numberOfKeys; id++, i++) {
        key_t k;
        if ((k = ftok(".", id)) == -1) {
            perror("message queue keys generation");
            exit(1);
        }
        placesKeys.push_back(k);
    }
    id++;
    for (int i = 0; i < 4; i++, id++) {
        key_t k;
        if ((k = ftok(".", id)) == -1) {
            perror("semaphore key generation");
            exit(1);
        }
        semKeys[i] = k;
    }
}

void getPersonInfoID() {
    if ((processInfoID = msgget(placesKeys[0], IPC_CREAT | 0777)) == -1) {
        perror("new locations message queue creation");
        exit(1);
    }
}

void startOpenGL() {
    glClearColor(1, 1, 1, 1);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 800);
    glutInitWindowPosition(1000, 100);
    glutCreateWindow("Occupation Interior Ministry");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timer, 0);
    glClearColor(1, 1, 1, 1);
    glutMainLoop();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    displayTime();
    displayStatistics();
    displayEntrance();
    displayQueue();
    displayStandingUp();
    displayItems();
    displayOfficesNames();
    glFlush();
    glutSwapBuffers();
}

void displayTime() {
    glColor3f(0, 0, 0);
    char *temp = (char *) malloc(sizeof(int)*3);
    if(m>=10 && s>=10)
        sprintf(temp, "%d:%d:%d", h, m, s);
    else if(m<10 && s>=10)
        sprintf(temp, "%d:0%d:%d", h, m, s);
    else if(m>=10 && s<10)
        sprintf(temp, "%d:%d:0%d", h, m, s);
    else
        sprintf(temp, "%d:0%d:0%d", h, m, s);
    renderBitMap(87, 95, GLUT_BITMAP_TIMES_ROMAN_24, temp);
}

void displayEntrance() {
    glColor3f(0, 0, 0);
    char *temp = (char *) malloc(sizeof(int));
    sprintf(temp, "People : %ld", people[0].size());
    renderBitMap(3, 8, GLUT_BITMAP_TIMES_ROMAN_24, temp);
}

void displayQueue() {
    glColor3f(0, 0, 0);
    char *males = (char *) malloc(sizeof(int));
    char *females = (char *) malloc(sizeof(int));
    sprintf(males, "Males Queue: %ld", people[1].size());
    renderBitMap(4, 94, GLUT_BITMAP_TIMES_ROMAN_24, males);
    sprintf(females, "Females Queue: %ld", people[2].size());
    renderBitMap(14, 84, GLUT_BITMAP_TIMES_ROMAN_24, females);
}

void displayStandingUp(){
    glColor3f(0, 0, 0);
    char *standingUp = (char *) malloc(sizeof(int));
    int standing;
    if(people[5].size() > INITIAL_CHAIR)
        standing = people[5].size() - INITIAL_CHAIR;
    else
        standing = 0;
    sprintf(standingUp, "People Standing Up: %d", standing);
    renderBitMap(25, 5, GLUT_BITMAP_TIMES_ROMAN_24, standingUp);
}

void displayOfficesNames(){
    char *birth = (char*)"BIRTH CERTIFICATE";
    char *travel = (char*)"TRAVEL DOCUMENT";
    char *family = (char*)"FAMILY REUNION";
    char *idd = (char*)"ID-RELATED";
    glColor3f(0, 0, 0);
    renderBitMap(66, 2, GLUT_BITMAP_TIMES_ROMAN_24, birth);
    renderBitMap(66, 22, GLUT_BITMAP_TIMES_ROMAN_24, travel);
    renderBitMap(66, 42, GLUT_BITMAP_TIMES_ROMAN_24, family);
    renderBitMap(66, 62, GLUT_BITMAP_TIMES_ROMAN_24, idd);
}

void displayStatistics() {
    char *unserved = (char *) malloc(sizeof(int));
    char *unhappy = (char *) malloc(sizeof(int));
    char *satisfied = (char *) malloc(sizeof(int));
    sprintf(unserved, "UNSERVED: %d", UNSERVED);
    sprintf(unhappy, "UNHAPPY: %d", UNHAPPY);
    sprintf(satisfied, "SATISFIED: %d", SATISFIED);
    glColor3f(0, 0, 0);
    renderBitMap(60, 94, GLUT_BITMAP_TIMES_ROMAN_24, unserved);
    renderBitMap(60, 88, GLUT_BITMAP_TIMES_ROMAN_24, unhappy);
    renderBitMap(60, 82, GLUT_BITMAP_TIMES_ROMAN_24, satisfied);
}

void renderBitMap(double x, double y, void *font, char *string) {
    char *c;
    glRasterPos2f(x, y);
    for (c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

void reshape(int w, int he) {
    glViewport(0, 0, (GLsizei) w, (GLsizei) he);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 100, 0, 100);
    glMatrixMode(GL_MODELVIEW);
}

void timer(int) {
    glutPostRedisplay();
    glutTimerFunc(1, timer, 0);
    checkEnd();
    if (s < (60 - secRate)) {
        s += secRate;
    }
    else {
        s = 0;
        if (m < (60 - minRate))
            m += minRate;
        else {
            m = 0;
            if (h < 23)
                h++;
            else
                h = 0;
        }
    }

    if (h == 8 && m == 0 && s == 0)
        kill(placesID[0], SIGUSR1);

    if (h == 15 && m == 0 && s == 0)
        kill(placesID[0], SIGUSR1);

    if (h == 23 && m == 0 && s == 0)
        termFlag = true;

    if (h < 13)
        createNewPerson();

    checkNewLocations();
}

void displayItems() {
    glBegin(GL_POLYGON);
    glColor3f(1, 1, 0);
    glVertex2f(0, 20);
    glVertex2f(20, 20);
    glVertex2f(20, 22);
    glVertex2f(0, 22);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(1, 1, 0);
    glVertex2f(20, 0);
    glVertex2f(22, 0);
    glVertex2f(22, 22);
    glVertex2f(20, 22);
    glEnd();
    for (int i = 0; i < 3; i++) {
        glBegin(GL_POLYGON);
        glColor3f(1, 0, 0);
        glVertex2f(10 * i, 22);
        glVertex2f((10 * i) + 2, 22);
        glVertex2f((10 * i) + 2, 100 - (10 * i));
        glVertex2f(10 * i, 100 - (10 * i));
        glEnd();
    }
    for (int i = 0; i < 3; i++) {
        glBegin(GL_POLYGON);
        glColor3f(1, 0, 0);
        glVertex2f(10 * i, 100 - (10 * i));
        glVertex2f(37, 100 - (10 * i));
        glVertex2f(37, 100 - (10 * i) - 2);
        glVertex2f(10 * i, 100 - (10 * i) - 2);
        glEnd();
    }
    for (int i = 0; i < 3; i++) {
        glBegin(GL_POLYGON);
        if (i == 0 && metalMaleLight > 0) {
            glColor3f(1, 0, 0);
        } else if (i == 2 && metalFemaleLight > 0) {
            glColor3f(1, 0, 0);
        } else
            glColor3f(0, 0, 0);
        glVertex2f(37, 100 - (10 * i));
        glVertex2f(45, 100 - (10 * i));
        glVertex2f(45, 100 - (10 * i) - 2);
        glVertex2f(37, 100 - (10 * i) - 2);
        glEnd();
    }
    for (int i = 0; i < 2; i++) {
        glBegin(GL_POLYGON);
        if (i == 0 && metalMaleLight > 0) {
            glColor3f(1, 0, 0);
        } else if (i == 1 && metalFemaleLight > 0) {
            glColor3f(1, 0, 0);
        } else
            glColor3f(0, 0, 0);
        glVertex2f(39, 100 - (10 * i) - 2);
        glVertex2f(41, 100 - (10 * i) - 2);
        glVertex2f(41, 90 - (10 * i));
        glVertex2f(39, 90 - (10 * i));
        glEnd();
    }
    for (int i = 0; i < 2; i++) {
        glBegin(GL_POLYGON);
        if (i == 0 && metalMaleLight > 0) {
            glColor3f(1, 0, 0);
            metalMaleLight--;
        } else if (i == 1 && metalFemaleLight > 0) {
            glColor3f(1, 0, 0);
            metalFemaleLight--;
        } else
            glColor3f(0, 0, 0);
        glVertex2f(43, 100 - (10 * i));
        glVertex2f(45, 100 - (10 * i));
        glVertex2f(45, 100 - (10 * i) - 5);
        glVertex2f(43, 100 - (10 * i) - 5);
        glEnd();
        glBegin(GL_POLYGON);
        glColor3f(0, 0, 0);
        glVertex2f(43, 100 - (10 * i) - 7);
        glVertex2f(45, 100 - (10 * i) - 7);
        glVertex2f(45, 100 - (10 * i) - 10);
        glVertex2f(43, 100 - (10 * i) - 10);
        glEnd();
    }
    glBegin(GL_POLYGON);
    glColor3f(1, 1, 0);
    glVertex2f(45, 98);
    glVertex2f(100, 98);
    glVertex2f(100, 100);
    glVertex2f(45,100);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(1, 1, 0);
    glVertex2f(98, 100);
    glVertex2f(100, 100);
    glVertex2f(100, 80);
    glVertex2f(98,80);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(1, 1, 0);
    glVertex2f(22, 0);
    glVertex2f(65, 0);
    glVertex2f(65, 2);
    glVertex2f(22,2);
    glEnd();
    for (int i = 0; i < 2; i++) {
        glBegin(GL_POLYGON);
        glColor3f(0, 0, 0);
        glVertex2f((10 * i) + 5, 20);
        glVertex2f((10 * i) + 7, 20);
        glVertex2f((10 * i) + 7, 28);
        glVertex2f((10 * i) + 5, 28);
        glEnd();
    }
    for (int i = 0; i < 2; i++) {
        glBegin(GL_POLYGON);
        glColor3f(0, 0, 0);
        glVertex2f((10 * i) + 2, 23);
        glVertex2f((10 * i) + 10, 23);
        glVertex2f((10 * i) + 10, 25);
        glVertex2f((10 * i) + 2, 25);
        glEnd();
    }
    for (int i = 0; i < INITIAL_CHAIR; i++) {
        for (int j = 0; j < 22 && j + (i * 22) < INITIAL_CHAIR; j++) {
            glBegin(GL_POLYGON);
            if (j + (i * 23) < people[5].size())
                glColor3f(1, 0, 0);
            else
                glColor3f(1, 1, 0);
            glVertex2f(55 - (5 * i), 75 - (3 * j));
            glVertex2f(54 - (5 * i), 75 - (3 * j));
            glVertex2f(54 - (5 * i), 74 - (3 * j));
            glVertex2f(55 - (5 * i), 74 - (3 * j));
            glEnd();
        }
    }
    for (int i = 0; i < 4; i++) {
        glBegin(GL_POLYGON);
        switch (i) {
            case 0:
                glColor3f(0, 1, 0);
                break;
            case 1:
                glColor3f(0, 0, 1);
                break;
            case 2:
                glColor3f(0, 1, 1);
                break;
            default:
                glColor3f(1, 0, 1);
        }
        glVertex2f(65, 20 * i);
        glVertex2f(100, 20 * i);
        glVertex2f(100, 20 * (1 + i));
        glVertex2f(65, 20 * (1 + i));
        glEnd();
        glBegin(GL_POLYGON);
        glColor3f(1, 1, 1);
        glVertex2f(65, (20 * i) + 6);
        glVertex2f(90, (20 * i) + 6);
        glVertex2f(90, (20 * i) + 14);
        glVertex2f(65, (20 * i) + 14);
        glEnd();
        int stand = 71;
        for (int j = 0; j < numOfWindows[i]; j++) {
            glBegin(GL_POLYGON);
            if (j < people[i + 6].size()) {
                glColor3f(1, 0, 0);
            } else
                glColor3f(1, 1, 0);
            glVertex2f(stand, (20 * i) + 16);
            glVertex2f(stand + 2, (20 * i) + 16);
            glVertex2f(stand + 2, (20 * i) + 14);
            glVertex2f(stand, (20 * i) + 14);
            glEnd();
            stand += 4;
        }
    }
}

void checkNewLocations() { /*check if two or more cause problem*/
    CHANGE change;
    struct msqid_ds bufLoc;
    msgctl(processInfoID, IPC_STAT, &bufLoc);
    if (bufLoc.msg_qnum != 0) {
        if ((msgrcv(processInfoID, &change, sizeof(change), 1, 0)) == -1) {
            perror("new locations message queue reciving meesage");
            exit(4);
        }
        if (change.loc == 5 || change.loc == 6) {
            //cout << "waiting room case" << endl;
            int previous = 3;
            if (change.loc == 6)
                previous = 4;
            people[5][change.pid] = people[previous][change.pid];
            people[previous].erase(change.pid);
        } 
        else if (change.loc == 2 || change.loc == 1) {
            //cout << "queue case" << endl;
            people[change.loc][change.pid] = people[0][change.pid];
            people[0].erase(change.pid);
        } 
        else if (change.loc >= 7 && change.loc <= 10) {
            //cout << "office case" << endl;
            people[change.loc - 1][change.pid] = people[5][change.pid];
            people[5].erase(change.pid);
        } 
        else if (change.loc > 10 && change.loc < 19) {
            int caser = change.loc;
            if(caser>14)
                caser-=4;
            //cout << "leaving case" << endl;
            people[caser - 5].erase(change.pid);
            if(change.loc > 14)
                SATISFIED++;
            else UNHAPPY++;
            
        } 
        else if(change.loc == 3 || change.loc == 4){
            //cout << "metal detector case" << endl;
            if (change.loc == 3)
                metalMaleLight = 5;
            else
                metalFemaleLight = 5;
            people[change.loc][change.pid] = people[change.loc - 2][change.pid];
            people[change.loc - 2].erase(change.pid);
        }
        else if(change.loc == 21){
            people[0].erase(change.pid);
        }
        else if(change.loc == 23 || change.loc == 24){
            people[change.loc - 20].erase(change.pid);
        }
        else if (change.loc == 25 || change.loc == 26) {
            int previous = 3;
            if (change.loc == 26)
                previous = 4;
            people[previous].erase(change.pid);
        }
        else 
            cout << "change location error" << endl;
    }
}


void createNewPerson() {
    if (s%5 == 0 && totalNumOfPeople < MAX_NUMBER_OF_PEOPLE) {
        pid_t pid;
        seed++;
        int persantage = generate_Randoms(0, 99, getpid() + seed);
        if (persantage >= (100 - ARRIVAL_RATE)) {
            if ((pid = fork()) != -1) {
                if (pid == 0) {
                    int patience_level = generate_Randoms(PATIENCE_RANGE_LOWER, PATIENCE_RANGE_UPPER, getpid());
                    char *patience = (char *) malloc(sizeof(int));
                    sprintf(patience, "%d", patience_level);

                    char *patienceFreq = (char *) malloc(sizeof(int));
                    sprintf(patienceFreq, "%d", PATIENCE_FREQ);

                    const char *programName = "./child";
                    execl(programName, programName, patience, patienceFreq, NULL);
                }
                usleep(5000);
                PERSON person = defineNewPerson(pid);
                person.type = 1;
                people[0][pid] = person;
                allPeople.push_back(person);
                totalNumOfPeople++;

                if (msgsnd(msgID, &person, sizeof(person), 0) == -1) {
                    perror("sending a person to the entrance");
                    exit(4);
                }
            } 
            else {
                perror("Can't create more people");
            }
        }
    }
}

PERSON defineNewPerson(pid_t pid) {
    PERSON person;
    person.pid = pid;
    person.hour = h;
    person.min = m;
    person.sec = s;
    srand(time(NULL) % pid);
    int chooser = rand() % 2;
    if (chooser % 2 == 1)
        person.gender = 'M';
    else
        person.gender = 'F';
    chooser = generate_Randoms(0, 3, pid);
    person.paperType = offices[chooser];
    int patience_level = generate_Randoms(PATIENCE_RANGE_LOWER, PATIENCE_RANGE_UPPER, getpid());
    person.patience = patience_level;
    srand(time(NULL) % pid + 1);
    person.priority = rand() % 6 + (100 / (person.hour + person.min / 60 + person.sec / 3600));
    if ((person.key = ftok(".", '1')) == -1) {
        perror("person's key generation");
        exit(1);
    }
    return person;
}

void initializePlaces() {
    // Run the entrace with the queues
    char *processInfoIDSTR = (char *) malloc(sizeof(int));
    sprintf(processInfoIDSTR, "%d", processInfoID);
    initializeEntrance(processInfoIDSTR);
    // run the dectector
    char *tempNextKey = (char *) malloc(sizeof(key_t));
    sprintf(tempNextKey, "%d", placesKeys[4]);
    initializeMetalDet(processInfoIDSTR, tempNextKey);
    // run the waiting room
    initializeWaitingRoom(processInfoIDSTR, tempNextKey);
    // run officies windows
    for (int i = 0; i < 4; i++) {
        char *temp = (char *) malloc(sizeof(key_t));
        sprintf(temp, "%d", placesKeys[5 + i]);
        char temp2[1] = {offices[i]};
        char *temp3 = (char *) malloc(sizeof(key_t));
        sprintf(temp3, "%d", semKeys[i]);
        for (int j = 0; j < numOfWindows[i]; j++) {
            initializeOffices(processInfoIDSTR, temp, temp2, temp3, j);
        }
    }
}

void initializeEntrance(char *temp4) {
    char *temp = (char *) malloc(sizeof(key_t));
    sprintf(temp, "%d", placesKeys[1]);

    char *temp2 = (char *) malloc(sizeof(key_t));
    sprintf(temp2, "%d", placesKeys[2]);

    char *temp3 = (char *) malloc(sizeof(key_t));
    sprintf(temp3, "%d", placesKeys[3]);

    char *lower = (char *) malloc(sizeof(int));
    sprintf(lower, "%d", LOWER_QUEUE_THRESH);

    char *upper = (char *) malloc(sizeof(int));
    sprintf(upper, "%d", UPPER_QUEUE_THRESH);

    pid_t pid = fork();
    if (pid != -1) {
        if (pid != 0)
            placesID.push_back(pid);
        else if (pid == 0) {
            const char *programName = "./entrance";
            execl(programName, programName, temp, temp2, temp3, temp4, upper, lower, NULL);
        }
    } 
    else {
        perror("fork while creating entrance");
        exit(1);
    }
}

void initializeMetalDet(char *temp4, char *tempNextKey) {
    for (int i = 0; i < 2; i++) {
        pid_t pid = fork();

        char *tempKey = (char *) malloc(sizeof(key_t));
        sprintf(tempKey, "%d", placesKeys[2 + i]);

        char *metalLower = (char *) malloc(sizeof(int));
        sprintf(metalLower, "%d", METAL_LOWER);

        char *metalUpper = (char *) malloc(sizeof(int));
        sprintf(metalUpper, "%d", METAL_UPPER);

        char *gender = (char *)malloc(sizeof(char));
        if (i == 1)
            gender[0] = 'F';
        else
            gender[0] = 'M';
        
        if (pid != -1) {
            if(pid != 0)
                placesID.push_back(pid);
            else if (pid == 0) {
                const char *programName = "./metalDet";
                execl(programName, programName, tempKey, gender, tempNextKey, temp4, metalLower, metalUpper, NULL);
            }
        } 
        else {
            perror("fork while creating metal detector");
            exit(1);
        }
    }
}

void initializeWaitingRoom(char *temp4, char *tempNextKey) {

    char *officeKeys[4];
    char *window[4];
    char *waitingChairs = (char *) malloc(sizeof(int));
    sprintf(waitingChairs, "%d", INITIAL_CHAIR);

    for(int i=0; i<4; i++) {
        window[i] = (char *) malloc(sizeof(int));
        officeKeys[i] = (char *) malloc(sizeof(key_t));
        sprintf(window[i], "%d", numOfWindows[i]);
        sprintf(officeKeys[i], "%d", placesKeys[i+5]);
    }
    
    pid_t pid = fork();
    
    if (pid != -1) {
        if(pid != 0)
            placesID.push_back(pid);
        else if (pid == 0) {
            const char *programname = "./waitingRoom";
            execl(programname, programname, tempNextKey, waitingChairs, officeKeys[0], officeKeys[1], officeKeys[2],
                  officeKeys[3], temp4, window[0], window[1], window[2], window[3], NULL);
        }
    } 
    else {
        perror("fork while creating waiting room");
        exit(1);
    }
}

void initializeOffices(char *temp4, char *officeKey, char *type, char *semaKey, int ord) {
    pid_t pid = fork();
    char *order = (char *) malloc(sizeof(int));
    sprintf(order, "%d", ord);
    char *officeLowerStr = (char *) malloc(sizeof(int));
    sprintf(officeLowerStr, "%d", OFFICE_LOWER);
    char *officeUpperStr = (char *) malloc(sizeof(int));
    sprintf(officeUpperStr, "%d", OFFICE_UPPER);
    
    if (pid != -1) {
        if(pid != 0)
            placesID.push_back(pid);
        else if (pid == 0) {
            const char *programname = "./office";
            execl(programname, programname, officeKey, type, semaKey, temp4, order, officeUpperStr, officeLowerStr, NULL);
        }
    } 
    else {
        perror("fork while creating office");
        exit(1);
    }
}

void generateNumberOfTeller() {
    int size = sizeof(options) / sizeof(int);
    int seed_2 = 0;
    srand(time(NULL) % getpid() + seed_2);
    for (int i = 0; i < size; i++) {
        srand(time(NULL) % getpid() + seed_2);
        seed++;
        int index = rand() % size;

        while (true) {
            if (numOfWindows[index] == 0) {
                numOfWindows[index] = options[i];
                break;
            } 
            else {
                seed_2++;
                srand(time(NULL) % getpid() + seed_2);
                index = rand() % size;
            }
        }
    }
}

void readFile() {
    string format[] = {"MAX_NUMBER_OF_PEOPLE", "QUEUE_THRESH", "OFFICE_THRESH", "PATIENCE_FREQ", "METAL_RANGE",
                       "INITIAL_TIME", "INITIAL_CHAIR", "TIME_INCREASE_RATE", "PATIENCE_RANGE", "ARRIVAL_RATE",
                       "TOTAL_NUMBER_OF_TELLERS", "TERMINATION_CASES"};
    string line;
    ifstream
    MyReadFile("arguments.txt");
    while (getline(MyReadFile, line)) {
        stringstream ss(line);
        vector <string> vec;
        string temp;
        while (getline(ss, temp, ' ')) {
            vec.push_back(temp);
        }
        if (format[0].compare(vec[vec.size() - 1]) == 0) {
            MAX_NUMBER_OF_PEOPLE = stoi(vec[vec.size() - 2]);
        } 
        else if (format[1].compare(vec[vec.size() - 1]) == 0) {
            UPPER_QUEUE_THRESH = stoi(vec[vec.size() - 2]);
            LOWER_QUEUE_THRESH = stoi(vec[vec.size() - 3]);
        } 
        else if (format[2].compare(vec[vec.size() - 1]) == 0) {
            OFFICE_UPPER = stoi(vec[vec.size() - 2]);
            OFFICE_LOWER = stoi(vec[vec.size() - 3]);
        } 
        else if (format[3].compare(vec[vec.size() - 1]) == 0) {
            PATIENCE_FREQ = stoi(vec[vec.size() - 2]);
        } 
        else if (format[4].compare(vec[vec.size() - 1]) == 0) {
            METAL_LOWER = stoi(vec[vec.size() - 3]);
            METAL_UPPER = stoi(vec[vec.size() - 2]);
        } 
        else if (format[5].compare(vec[vec.size() - 1]) == 0) {
            h = stoi(vec[vec.size() - 3]);
            m = stoi(vec[vec.size() - 2]);
        } 
        else if (format[6].compare(vec[vec.size() - 1]) == 0) {
            INITIAL_CHAIR = stoi(vec[vec.size() - 2]);
            if(INITIAL_CHAIR>132)
                INITIAL_CHAIR = 138;
        } 
        else if (format[7].compare(vec[vec.size() - 1]) == 0) {
            secRate = stoi(vec[vec.size() - 3]);
            minRate = stoi(vec[vec.size() - 2]);
        } 
        else if (format[8].compare(vec[vec.size() - 1]) == 0) {
            PATIENCE_RANGE_LOWER = stoi(vec[vec.size() - 3]);
            PATIENCE_RANGE_UPPER = stoi(vec[vec.size() - 2]);
        } 
        else if (format[9].compare(vec[vec.size() - 1]) == 0) {
            ARRIVAL_RATE = stoi(vec[vec.size() - 2]);
        } 
        else if (format[10].compare(vec[vec.size() - 1]) == 0) {
            options[0] = stoi(vec[vec.size() - 2]);
            options[1] = stoi(vec[vec.size() - 3]);
            options[2] = stoi(vec[vec.size() - 4]);
            options[3] = stoi(vec[vec.size() - 5]);
        }
        else if (format[11].compare(vec[vec.size() - 1]) == 0) {
            goalSATISFIED = stoi(vec[vec.size() - 4]);
            goalUNHAPPY = stoi(vec[vec.size() - 3]);
            goalUNSERVED = stoi(vec[vec.size() - 2]);
        }  
        else {
            perror("file error");
            exit(1);
        }
    }
}

void checkEnd(){
    if(SATISFIED == goalSATISFIED || UNHAPPY == goalUNHAPPY || UNSERVED == goalUNSERVED || termFlag){
        cout << "simulations ends now" << endl;
        raise(SIGINT);
        exit(0);
    }
}

void handle_SIGCHID(int sig, siginfo_t *info, void *context) {
    pid_t signalPid = info->si_pid;
    int status;
    waitpid(signalPid, &status, WNOHANG);
    if(WIFEXITED(status)){
        UNSERVED+=1;
    }
    return;
}

void killALL() {
    for (int i = 0; i < allPeople.size(); i++) {
        if(kill(allPeople[i].pid, 0) == 0){
            if(kill(allPeople[i].pid, SIGTERM) != 0){
                //perror("error with killing people process");
            }
        }
    }
    for (int i = 0; i < placesID.size(); i++) {
        if(kill(placesID[i], 0) == 0){
            if(kill(placesID[i], SIGTERM) != 0){
                perror("error with killing places process");
            }
        }
    }
    return;
}

void handle_SIGINT(int signum) {
    killALL();
    raise(SIGTERM);
    exit(0);
}

void removeAllQueuesSem() {
    system("ipcrm --all=msg");
    system("ipcrm --all=sem");
    return;
}