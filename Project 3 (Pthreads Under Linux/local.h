#include <GL/glut.h>
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <bits/stdc++.h>
#include <unistd.h>

using namespace std;

struct Chocolate{
    int index;
    char type;
    int status;
    bool lock;
};

struct Batch{
    int index;
    char type;
};

struct Box{
    int index;
    char type;
};