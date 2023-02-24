#include <iostream>
using namespace std;
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <pthread.h>
#include <vector>
#include <queue>
#include <unordered_set>
#include <sstream>
#include <fstream>
#include <time.h>

struct ROOM{
    int number;
    int max_people;
    bool taken;
};

struct TOURIST_GROUP{
    int order;
    int people;
    int state;
    ROOM room;
    int number_of_luggage;
};

struct LUGGAGE{
    TOURIST_GROUP owners;
};