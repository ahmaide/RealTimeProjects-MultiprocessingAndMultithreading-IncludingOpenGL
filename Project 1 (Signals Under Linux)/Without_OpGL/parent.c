#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <sys/signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#define NUM_OF_PLAYERS 10

void start_round();
void announce_winner();
void count_points();
void terminate_allChild(int pid_number[], int numOfChild);
void handle_sigusr1(int sig);
void handle_sigusr2(int sig);
void terminate_FIFO(int size, int size_2);
void manage_round();

pid_t pid[NUM_OF_PLAYERS];
int green_time = -1, red_time = -1;
int green_points = 0, red_points = 0;
int numOfRounds = 3;
char *parntFifo[] = {"parFifo_0", "parFifo_1"};
char *fifo[] = 
	{"myfifo0","myfifo1","myfifo2","myfifo3","myfifo4",
	"myfifo5","myfifo6","myfifo7","myfifo8","myfifo9"};

int main(){
	int size = sizeof(fifo)/sizeof(fifo[0]);
	int size_2 = sizeof(parntFifo)/sizeof(parntFifo[0]);
	int pid_number[NUM_OF_PLAYERS];
	int parent_pid = getpid();

	printf("Please enter the number of rounds for winning\n(enter 0 if you want to use the default number): ");
	int temp = 0;
	scanf("%d",&temp);
	while(temp < 0 ){
		printf("invalid input!\nPlease enter the number of roundsn for winning: ");
		scanf("%d",&temp);
	}

	if (temp != 0){
		numOfRounds = temp;
	}

	signal(SIGUSR1, handle_sigusr1);
	signal(SIGUSR2, handle_sigusr2);

	for(int i = 0; i<size; i++){
		if(mkfifo(fifo[i],0777) == -1){
			if(errno != EEXIST){
				printf("Error with creating FIFO\n");
				exit(-1);
			}
		}
	}

	for(int i = 0; i<size_2; i++){
		if(mkfifo(parntFifo[i],0777) == -1){
			if(errno != EEXIST){
				printf("Error with creating FIFO\n");
				exit(-2);
			}
		}
	}
	
	for(int i=0; i<NUM_OF_PLAYERS; i++){
		pid[i] = fork();
		usleep(500);
		if(pid[i] == -1){
			printf("Error with forking");
			exit(-1);
		}

		if(getpid() == parent_pid){
			pid_number[i] = pid[i]; // register children pids
		}
		
		char *str = malloc(sizeof(int));
		sprintf(str, "%d", i);

		char *str_rounds = malloc(sizeof(int)+1);
		sprintf(str_rounds, "%d", numOfRounds);

		if(pid[i] == 0 && i < 5){ // GREEN team
			if(i != 0){
				if (execl("./child","./child",str,"Green",fifo[i-1],fifo[i], str_rounds, NULL) == -1){
					printf("Error with Executing");
					exit(-1);
				}
			}
			else if (i == 0){ 
				if (execl("./child","./child",str,"Green",parntFifo[0],fifo[i], str_rounds, NULL) == -1){
					printf("Error with Executing");
					exit(-2);
				}
			}
			return 0;
		}
		else if (pid[i] == 0 && i >= 5){ // RED team
			if(i != 5){
				if(execl("./child","./child",str,"red",fifo[i-1],fifo[i], str_rounds, NULL) == -1){
					printf("Error with Executing");
					exit(-3);
				}
			}
			else if(i == 5){
				if(execl("./child","./child",str,"red",parntFifo[1],fifo[i], str_rounds, NULL) == -1){
					printf("Error with Executing");
					exit(-4);
				}
			}
			return 0;
		}
	}
	
	manage_round();
	int numOfChild = sizeof(pid_number)/sizeof(pid_number[0]);

	terminate_allChild(pid_number, numOfChild);
	announce_winner();
	terminate_FIFO(size, size_2);

	return 0;
}

void manage_round(){
	for(int i=0; i<numOfRounds*2-1; i++){
		green_time = -1;
		red_time = -1;
		usleep(500);
		start_round();	
		
		while(green_time == -1 || red_time == -1){
			sleep(1);
		}
		count_points();
		if(green_points == numOfRounds || red_points == numOfRounds)
			break;
	}
}

void terminate_FIFO(int size, int size_2){
	for(int i = 0; i<size; i++){
		if(remove(fifo[i]) != 0){
			printf("Error with closing fifo\n");
			exit(-1);	
		}
	}

	for(int i = 0; i<size_2; i++){
		if(remove(parntFifo[i]) != 0){
			printf("Error with closing fifo\n");
			exit(-2);		
		}
	}
}

void terminate_allChild(int pid_number[], int numOfChild){
	for(int i=0; i<numOfChild; i++){
		kill(pid_number[i], SIGTERM);
	}
	return;
}

void count_points(){
	if(green_time > red_time)
		red_points++;
	else if(green_time < red_time)
		green_points++;
	else if(green_time == red_time && green_time != -1){
		green_points++;
		red_points++;
	}
	else{
		printf("Error with counting points");
		exit(-1);
	}

	return;
}

void announce_winner(){
	printf("\nGreen Team: %d\tRed Team: %d\n", green_points, red_points);
	if(green_points < red_points)
		printf("\nThe winner is RED team\n");
	else if(green_points > red_points)
		printf("\nThe winner is GREEN team\n");
	else if(green_points == red_points)
		printf("\nTie !!\n");
	else{
		printf("Error with announcing winner");
		exit(-1);
	}

	return;
}

void start_round(){
	int fd = open(parntFifo[0], O_WRONLY);
	int fd2 = open(parntFifo[1], O_WRONLY);

	if(fd == -1 || fd2 == -1){
		printf("Error with opening fifo");
		exit(-1);
	}
	if(green_points != 0 || red_points != 0){
		printf("\nGreen Team: %d\tRed Team: %d\n\nThis Round has finished\n", green_points, red_points);
	}
	printf("Round Starts Now\nGO GO GO !!\n");
	close(fd);
	close(fd2);
	return ;
}

void handle_sigusr1(int sig){
	// red team;
	usleep(1000);
	printf("RED team has finished this round\n");
	red_time = time(NULL);
	return;
}

void handle_sigusr2(int sig){
	// green team
	usleep(1000);
	printf("GREEN team has finished this round\n");
	green_time = time(NULL);
	return;
}
