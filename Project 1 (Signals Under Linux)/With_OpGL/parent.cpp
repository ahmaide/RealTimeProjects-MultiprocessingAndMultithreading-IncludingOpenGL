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

#define NUM_OF_PLAYERS 10
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

using namespace std;

void renderbitmap(double x, double y, void *font, char *string);
void introscreen(int a, int x, int y);
void terminate_FIFO();
void announce_winner();
void terminate_allChild();
void start_round();
void readFile();
void teleport(vector<string>v);
void signal_catcherTeam1(int);
void signal_catcherTeam2(int);
int findMax(int arr[]);
int findMin(int arr[]);
void display();
void reshape(int w, int h);
void timer(int);
void transmit(int state, int team);
void init();
void displayScore();

vector<int>processes;
int rounds;
int redPoints=0;
int greenPoints=0;
double x;
double y;
int stateR =0;
int stateG =0;
int EdgeP;
int EdgeN;
int locs[6];
int speeds[NUM_OF_PLAYERS];
bool q = true;

const char* fifos[] = 
{"fifo1", "fifo2","fifo3","fifo4","fifo5","fifo6","fifo7","fifo8","fifo9","fifo10"};

int main(int argc, char **argv) {
    int pid = getpid();
    readFile();
    rounds = locs[5];
    EdgeP= findMax(locs);
    x = locs[0];
    y = locs[0];
    EdgeN = findMin(locs);
    int team;
    cout << "Number of rounds is " << rounds << endl;

    for(int i = 0; i<NUM_OF_PLAYERS; i++){
		if(mkfifo(fifos[i],0777) == -1){
			if(errno != EEXIST){
				printf("Error with creating FIFO\n");
				exit(-1);
			}
		}
	}
    
    for(int i=0; i<NUM_OF_PLAYERS ; i++){
        pid = fork();
        usleep(500);
        if(pid != 0){ // parent process
            processes.push_back(pid);
        }

        if(pid == -1){
			printf("Error with forking");
			exit(-1);
		}

        if(pid == 0){ // child process
            team = (i/5) + 1;
            vector<string> v;
            int nextP;
            if(i%5 == 0)
                nextP = 0;
            else
                nextP = processes[i-1];
            v.push_back(to_string(5-(i%5))); // save player information
            v.push_back(to_string(team));
            v.push_back(to_string(nextP));
            v.push_back(to_string(locs[4-(i%5)]));
            v.push_back(to_string(locs[4-((i+1)%5)]));
            v.push_back(fifos[(4-(i%5)) + (i/5)*5]);
            teleport(v);
        }
    }

    usleep(500);

    for(int i=0; i<NUM_OF_PLAYERS; i++){
        int fd = open(fifos[i], O_RDWR);
        if(read(fd, &speeds[i], sizeof(int)) == -1){
            cout << "Fifo file is not found :(" << endl;
            exit(-1);
        }
        close(fd);
    }

    glutInit(&argc, argv);
    init();
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(750, 750);
    glutInitWindowPosition(200, 100);
    glutCreateWindow("Green vs Red");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timer, 0);
    init();
    glutMainLoop();
    while(1);
    
    return 0;
}

void init(){
    glClearColor(1,1,1,1);
}

int findMax(int arr[]){
    int max = arr[0];
    int size = 5;
    cout << size << endl;
    for(int i=1; i<size; i++){
        if(arr[i] > max)
            max= arr[i];
    }
    return max;
}

int findMin(int arr[]){
    int min = arr[0];
    int size = 5;
    for(int i=1; i<size ; i++){
        if(arr[i] < min)
            min= arr[i];
    }
    return min;
}

void teleport(vector<string> v){
    const char *programname = "./child";
    const char **argv = new const char* [v.size()+2];
    argv [0] = programname;

    for (int j = 0;  j < v.size()+1; j++)
        argv [j+1] = v[j] .c_str();

    argv [v.size()+1] = NULL;
    execv (programname, (char **)argv);
    return;
}

void readFile(){
    string file("locations.txt");
    ifstream input_file(file);
    if (!input_file.is_open()) {
        cerr << "Could not open the file - '"
             << file<< "'" << endl;
        exit(-1);
    }
    int number;
    int i;
    for (i=0; input_file >> number && i<6 ; i++) {
        locs[i]=number;
    }

    if(i<5){
        cout << "You have some missing data in the locations file" << endl;
        exit(1);
    }
    else if(i==5){
        locs[5]=5;
    }
    input_file.close();
    return;
}

void announce_winner(){
    if(redPoints == rounds && redPoints != greenPoints){
        cout << ANSI_COLOR_RED "\nThe red team won the match !" ANSI_COLOR_RESET << endl;
    }
    else if(greenPoints == rounds && redPoints != greenPoints){
        cout << ANSI_COLOR_GREEN "\nThe green team won the match !" ANSI_COLOR_RESET << endl;
    }
    else if(greenPoints == rounds && redPoints == greenPoints){
        cout << "\nTie !!!" << endl;  
    }
    else return; // if the match didn't finish, skip the remaining lines

    terminate_FIFO();
    terminate_allChild();
    return ;
}

void transmit(int state, int team){
    if(state<4){
        if(team == 1)
            stateR+=1;
        else
            stateG+=1;
    }
    else{
        if(team == 1){
            if(q){
                redPoints += 1;
                cout << ANSI_COLOR_RED "RED team won this round" ANSI_COLOR_RESET << endl;
                cout << "GREEN : " << greenPoints << "\t RED : " << redPoints << endl;
                q=false;
            }
            else{
                announce_winner();
                start_round();
                stateR = 0;
                x = locs[0];
                stateG = 0;
                y = locs[0];
                q=true;
            }
        }
        else if(team == 2){
            if(q){
                greenPoints += 1;
                cout << ANSI_COLOR_GREEN "GREEN team won this round" ANSI_COLOR_RESET << endl;
                cout << "GREEN : " << greenPoints << "\t RED : " << redPoints << endl;
                q=false;
            }
            else{
                announce_winner();
                start_round();
                stateR = 0;
                x = locs[0];
                stateG = 0;
                y = locs[0];
                q=true;
            }
        }
    }
}

void start_round(){
    for(int i=0; i<NUM_OF_PLAYERS ; i++){
        kill(processes[i], SIGINT);
    }
    for(int i=0; i<NUM_OF_PLAYERS; i++){
        int fd = open(fifos[i], O_RDWR);
        if(read(fd, &speeds[i], sizeof(int)) == -1){
            cout << "File is not found :(" << endl;
            exit(-1);
        }
        close(fd);
    }
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    displayScore();
    for(int i=0; i<5; i++){
        introscreen(i +1, locs[i] + 2, EdgeP);
    }
    for(int i=0; i<5; i++){
        glBegin(GL_LINES);
        glColor3f(0,0,1);
        glVertex2f(locs[i],EdgeP+10);
        glVertex2f(locs[i],EdgeN-10);
    }
    for(int i=0; i<5 ; i++) {
        glBegin(GL_POLYGON);
        glColor3f(1, 1, 1);
        if(stateR==i){
            glVertex2f(x, (EdgeP - EdgeN)*0.5 +EdgeN + 5);
            glVertex2f(x, (EdgeP - EdgeN)*0.5 +EdgeN + 4);
            glVertex2f(x + 1, (((EdgeP - EdgeN)*0.5) +EdgeN) + 4);
            glVertex2f(x + 1, (((EdgeP - EdgeN)*0.5) +EdgeN) + 5);
            glEnd();
        }
        else{
            int j=i;
            if(stateR>i)
                j+=1;
            glVertex2f(locs[j%5], (((EdgeP - EdgeN)*0.5) +EdgeN) + 5);
            glVertex2f(locs[j%5], (((EdgeP - EdgeN)*0.5) +EdgeN) +4);
            glVertex2f(locs[j%5]+ 1, (((EdgeP - EdgeN)*0.5) +EdgeN) +4);
            glVertex2f(locs[j%5] + 1, (((EdgeP - EdgeN)*0.5) +EdgeN) + 5);
            glEnd();
        }
    }
    for(int i=0; i<5 ; i++) {
        glBegin(GL_POLYGON);
        glColor3f(1, 0, 0);
        if(stateR==i){
            glVertex2f(x, (EdgeP - EdgeN)*0.25 +EdgeN + 5 + i*3);
            glVertex2f(x, (EdgeP - EdgeN)*0.25 +EdgeN - 5);
            glVertex2f(x + 10, (((EdgeP - EdgeN)*0.25) +EdgeN) - 5);
            glVertex2f(x + 10, (((EdgeP - EdgeN)*0.25) +EdgeN) + 5 + i*3);
            glEnd();
        }
        else{
            int j=i;
            if(stateR>i)
                j+=1;
            glVertex2f(locs[j%5], (((EdgeP - EdgeN)*0.25) +EdgeN) + 5 + i*3);
            glVertex2f(locs[j%5], (((EdgeP - EdgeN)*0.25) +EdgeN) - 5);
            glVertex2f(locs[j%5]+ 10, (((EdgeP - EdgeN)*0.25) +EdgeN) - 5);
            glVertex2f(locs[j%5] + 10, (((EdgeP - EdgeN)*0.25) +EdgeN) + 5 + i*3);
            glEnd();
        }
    }
    for(int i=5; i<NUM_OF_PLAYERS ; i++) {
        glBegin(GL_POLYGON);
        glColor3f(0, 1, 0);
        if(stateG==i%5){
            glVertex2f(y, (EdgeP - EdgeN)*0.75 +EdgeN + 5 + (i%5)*3);
            glVertex2f(y, (EdgeP - EdgeN)*0.75 +EdgeN - 5);
            glVertex2f(y + 10, (EdgeP - EdgeN)*0.75 +EdgeN - 5);
            glVertex2f(y + 10, (EdgeP - EdgeN)*0.75 +EdgeN + 5 + (i%5)*3);
            glEnd();
        }
        else{
            int j=i;
            if(stateG>(i%5))
                j+=1;
            glVertex2f(locs[j%5], (EdgeP - EdgeN)*0.75 +EdgeN + 5 + (i%5)*3);
            glVertex2f(locs[j%5], (EdgeP - EdgeN)*0.75 +EdgeN - 5);
            glVertex2f(locs[j%5]+ 10, (EdgeP - EdgeN)*0.75 +EdgeN - 5);
            glVertex2f(locs[j%5] + 10, (EdgeP - EdgeN)*0.75 +EdgeN + 5 + (i%5)*3);
            glEnd();
        }
    }
    glFlush();
    glutSwapBuffers();
}

void introscreen(int a, int x, int y){
    glColor3f(1, 0, 1);
    char ac = a+48;
    char text[2] ={'A', ac};
    renderbitmap(x, y, GLUT_BITMAP_TIMES_ROMAN_10, text);
}

void displayScore(){
    glColor3f(0, 0, 0);
    char *temp = (char*)malloc(sizeof(int));
    sprintf(temp, "Red: %d", redPoints);
    char *temp_2 = (char*)malloc(sizeof(int));
    sprintf(temp_2, "    |   Green: %d", greenPoints);
    char *text = strcat(temp, temp_2);
    renderbitmap((EdgeP+EdgeN)/(2.7), (EdgeP+EdgeN)/2, GLUT_BITMAP_TIMES_ROMAN_24, text);
}

void renderbitmap(double x, double y, void *font, char *string){
    char *c;
    glRasterPos2f(x,y);
    for(c=string; *c!='\0';c++){
        glutBitmapCharacter(font, *c);
    }
}


void reshape(int w, int h){
    glViewport(0,0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(EdgeN - 10,EdgeP + 10,EdgeN - 10,EdgeP + 10);
    glMatrixMode(GL_MODELVIEW);
}

void timer(int){
    double scale = 0.2;
    glutPostRedisplay();
    glutTimerFunc(1000/60,timer,0);
    if(locs[stateR] < locs[(stateR+1)%5]){
        if(x>locs[(stateR+1)%5]){
            transmit(stateR, 1);
        }
        else{
            x+=scale*speeds[stateR];
        }
    }
    else{
        if(x<locs[(stateR+1)%5]){
            transmit(stateR, 1);
        }
        else{
            x-=scale*speeds[stateR];
        }
    }

    if(locs[stateG] < locs[(stateG+1)%5]){
        if(y>locs[(stateG+1)%5]){
            transmit(stateG, 2);
        }
        else{
            y+=scale*speeds[stateG+5];
        }
    }
    else{
        if(y<locs[(stateG+1)%5]){
            transmit(stateG, 2);
        }
        else{
            y-=scale*speeds[stateG+5];
        }
    }
}

void terminate_allChild(){
	for(int i=0; i<NUM_OF_PLAYERS; i++){
		kill(processes[i], SIGTERM);
	}
    sleep(2);
    exit(0);
}

void terminate_FIFO(){
	for(int i = 0; i<NUM_OF_PLAYERS; i++){ // remove all fifo files and exit
		if(remove(fifos[i]) != 0){
			printf("Error with closing fifo\n");
			exit(-1);	
		}
	}
	return;
}

