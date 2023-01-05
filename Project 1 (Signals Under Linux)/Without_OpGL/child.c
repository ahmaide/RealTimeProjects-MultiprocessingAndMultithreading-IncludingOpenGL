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

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define UPPER_TIME_LIMIT 6
#define LOWER_TIME_LIMIT 2

int wait_race_read(char *fifo);
int wait_race_write(char *fifo);

int main(int argc, char *argv[]){
	int index = atoi(argv[1]);
	char *team = argv[2];
	char *file_0 = argv[3];
	char *file_1 = argv[4];
	int numOfRounds = atoi(argv[5]);

	for(int i=0; i<numOfRounds*2 - 1; i++){
		srand(time(NULL)%getpid());
		int t = rand()%UPPER_TIME_LIMIT + LOWER_TIME_LIMIT + i/3;
		
		wait_race_read(file_0);
		sleep(t);

		if(strcmp(file_1,"myfifo9") != 0 && strcmp(file_1,"myfifo4") != 0)
			wait_race_write(file_1);
		else{
			if(strcmp(team, "red") == 0)
				kill(getppid(),SIGUSR1);
			else kill(getppid(),SIGUSR2);
		}

		int location = 0;
		if(strcmp(team, "red") == 0){
			location = index-3;
			if(location == 6){
				location = 1;
			}
			printf(ANSI_COLOR_RED"member %d of %s team reached location A_%d in : %d sec\n"ANSI_COLOR_RESET,index-4,team,location,t );
			fflush(stdout);
		}
		else {
			location = index+2;
			if(location == 6){
				location = 1;
			}
			printf(ANSI_COLOR_GREEN"member %d of %s team eached location A_%d in : %d sec\n"ANSI_COLOR_RESET,index+1,team,location,t);
			fflush(stdout);
		}
		
	}
	return 0;
}

int wait_race_read(char *fifo){
	int fd = open(fifo, O_RDONLY);
	if(fd == -1){
		printf("Error with fifo read");
		exit(-1);
	}
	close(fd);
	return 0;
}

int wait_race_write(char *fifo){
	int fd = open(fifo, O_WRONLY);
	if(fd == -1){
		printf("Error with fifo write");
		exit(-1);
	}
	close(fd);
	return 0;
}