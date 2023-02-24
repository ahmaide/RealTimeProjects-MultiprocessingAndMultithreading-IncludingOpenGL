#include <vector>
#include <sstream>
#include <fstream>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <map>
#include "local.h"
#include <unistd.h>

using namespace std;

void* endSimulation_Products(void* arg);
void* endSimulation_Time(void* arg);
void readFile();
void* manufactureTypeA(void* data);
void* manufactureTypeB(void* data);
void* manufactureTypeC(void* data);
void *move_to_expiration_date(void* data);
void *print_expiration_date(void *args);
void *separate_chocolates(void *arg);
void *prepare_carton_boxes(void *arg);
void startOpenGl();
void renderBitMap(double x, double y, void *font, char *string);
void display();
void displayPlacesNames();
void DatePrinterDisplay();
void ContainerDisplay();
void warehouseDisplay();
void truckDisplay();
void reshape(int w, int h);
void timer(int);
void rectangle(int x1, int y1, int x2, int y2, int r, int g, int b);
void* controlFactory(void* arg);
int generate_Randoms(int lower, int upper);
void* collectToStorageArea(void* arg);
void* collectToTruck(void* arg);
void* sendShipment(void* arg);
void* checkTrucks(void* arg);
void exit_program();

int currStored = 0, state = 0;
int x1[2] = {39, 39};
string fileName;
int minC, minB, minA;
int maxC, maxB, maxA;
int expDateTime, upperStore, lowerStore;
int targetA, targetB, targetC;
int termTime, storeTime, shippingRate;
int truckSize[3];
int numOfLinesTypeA, numOfLinesTypeB,numOfLinesTypeC;
int numOfWorkersA=8, numOfWorkersB=6, numOfWorkersC=5;
int batchSize, seed = 0;
int empCounter=0, empCounter2=0, empCounter3=0;

/// Production Lines -----------------------------
bool busyLineA[3][8];
bool busyLineB[2][6];
bool busyLineC[2][5];
vector<Chocolate> doneLineA[3][3];
vector<Chocolate> doneLineB[2][5];
vector<Chocolate> doneLineC[2][2];
map<int, Chocolate> randomA[3];
map<int, Chocolate> randomC[2];
vector<Chocolate> readyFromLines[3];
/// Printer ----------------------------
vector<Chocolate> batches_to_print[3];
vector<Chocolate> printed_batches[3];
int employees_available = 2;
int emplyeeDistance1[2] = {0};
int emplyeeState1[2] = {0};
/// Container --------------------------
vector<Chocolate> containers[3];
vector<Box> carton_boxes[3];
int storageArea[3] = {0};
int produced[3];
double emplyeeDistance2[3] = {0};
int emplyeeState2[3] = {0};
int emplyeeDistance3[3] = {0};
int emplyeeState3[3] = {0};
int separator_employees_available = 3;
int carton_employees_available = 3;
int fillEmployeeAva = 2;
bool flag = true;
// Mutex for lines turns
/// Warehouse --------------------------------------------
int emplyeeDistance4[2] = {0};
int emplyeeState4[2] = {0};
int availableTruck=0;
int emplyeeDistance5x[2] = {0};
int emplyeeDistance5y[2] = {0};
int emplyeeState5[2] = {0};
/// Trucks -----------------------------------------------
int truckBoxes[3][3];
int currentTruck=0;
int truckLevel[3] = {3, 30 ,57};
int truckState[3] = {0};
int truckDistance[3] = {0};

// Mutex for protecting access to the finished products queue
pthread_mutex_t lineA_mutex[3] = {
    PTHREAD_MUTEX_INITIALIZER, 
    PTHREAD_MUTEX_INITIALIZER, 
    PTHREAD_MUTEX_INITIALIZER
};

pthread_mutex_t lineC_mutex[2]{
    PTHREAD_MUTEX_INITIALIZER, 
    PTHREAD_MUTEX_INITIALIZER
};

pthread_mutex_t 
batches_mutex = PTHREAD_MUTEX_INITIALIZER,
containers_mutex = PTHREAD_MUTEX_INITIALIZER,
fillStorageEmployee_Mutex = PTHREAD_MUTEX_INITIALIZER,
carton_mutex = PTHREAD_MUTEX_INITIALIZER,
separator_employee_mutex = PTHREAD_MUTEX_INITIALIZER,
carton_employee_mutex = PTHREAD_MUTEX_INITIALIZER,
finished_products_mutex = PTHREAD_MUTEX_INITIALIZER,
suspend_mutex = PTHREAD_MUTEX_INITIALIZER,
sendToTruck_mutex = PTHREAD_MUTEX_INITIALIZER,
ready_mutex = PTHREAD_MUTEX_INITIALIZER,
employee_mutex = PTHREAD_MUTEX_INITIALIZER,
truck_mutex[2] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};

pthread_cond_t pauseFactory = PTHREAD_COND_INITIALIZER;

void init(){
    glClearColor(1,1,1,1);
}

int main(int argc, char** argv){
    fileName = argv[1];
    readFile();

    pthread_t manufacturing_linesA[numOfLinesTypeA*numOfWorkersA];
    pthread_t manufacturing_linesB[numOfLinesTypeB*numOfWorkersB];
    pthread_t manufacturing_linesC[numOfLinesTypeC*numOfWorkersC];
    pthread_t printing_employee[2];
    pthread_t print_thread, timeEndThread, productEndThread, controlThread;
    pthread_t separator_thread[3], carton_thread[3], storage_thread[2];
    pthread_t send_to_truck[2];
    pthread_t truck[3];
    pthread_t truckChecker;

    for (int i = 0; i < numOfLinesTypeA*numOfWorkersA; i++) {
        pthread_create(&manufacturing_linesA[i], nullptr, manufactureTypeA, (void *) &i);
        usleep(50000);
    }

    for (int i = 0; i < numOfLinesTypeB*numOfWorkersB; i++) {
        pthread_create(&manufacturing_linesB[i], nullptr, manufactureTypeB, (void *) &i);
        usleep(50000);
    }
    for (int i = 0; i < numOfLinesTypeC*numOfWorkersC ; i++) {
        pthread_create(&manufacturing_linesC[i], nullptr, manufactureTypeC, (void *) &i);
        usleep(50000);
    }

    for(int i = 0; i<2; i++){
        pthread_create(&printing_employee[i], nullptr, move_to_expiration_date, (void *) &i);
    }
    pthread_create(&timeEndThread, NULL, endSimulation_Time, NULL);
    pthread_create(&productEndThread, NULL, endSimulation_Products, NULL);
    pthread_create(&print_thread, NULL, &print_expiration_date, NULL);
    pthread_create(&controlThread, NULL, controlFactory, NULL);

    for(int i=0 ; i<3 ; i++){
        pthread_create(&separator_thread[i], NULL, &separate_chocolates, NULL);
        usleep(500);
        pthread_create(&carton_thread[i], NULL, &prepare_carton_boxes, NULL);
    }

    for(int i=0 ; i<2 ; i++){
        pthread_create(&storage_thread[i], NULL, &collectToStorageArea, (void *) &i);
        usleep(50000);
    }

    for(int i=0 ; i<2 ; i++){
        pthread_create(&send_to_truck[i], NULL, &collectToTruck, (void *) &i);
        usleep(50000);
    }

    for(int i=0; i<3 ; i++){
        pthread_create(&truck[i], NULL, &sendShipment, (void *) &i);
        usleep(50000);
    }

    pthread_create(&truckChecker, NULL, &checkTrucks, NULL);

    glutInit(&argc, argv);
    startOpenGl();
    return 0;
}

void exit_program(){
    pthread_mutex_destroy(&batches_mutex);
    pthread_mutex_destroy(&containers_mutex);
    pthread_mutex_destroy(&fillStorageEmployee_Mutex);
    pthread_mutex_destroy(&carton_mutex);
    pthread_mutex_destroy(&separator_employee_mutex);
    pthread_mutex_destroy(&carton_employee_mutex);
    pthread_mutex_destroy(&finished_products_mutex);
    pthread_mutex_destroy(&suspend_mutex);
    pthread_mutex_destroy(&sendToTruck_mutex);
    pthread_mutex_destroy(&ready_mutex);
    pthread_mutex_destroy(&employee_mutex);

    for(int i=0; i < 3; i++){
        pthread_mutex_destroy(&lineA_mutex[i]);
    }
    for(int i=0; i < 2; i++){
        pthread_mutex_destroy(&lineC_mutex[i]);
    }
    
    raise(SIGTERM);
}

void* endSimulation_Products(void* arg){
    int producedA, producedB, producedC;
    while(true){
        sleep(1);
        if(targetA <= produced[0] || targetB <= produced[1] ||targetC <= produced[2]){
            cout << "Prdoducts Reached The Target" << endl;
            sleep(1);
            exit_program();
        }
    }
    return NULL;
}

void* endSimulation_Time(void* arg){
    sleep(termTime*60);
    cout << "Simulation Ends Now" << endl;
    exit_program();
    return NULL;
}

void* controlFactory(void* arg){
    while(true){
        for(int i=0; i<3; i++){
            currStored += storageArea[i];
        }
        if (currStored <= lowerStore && !flag) {
            pthread_cond_broadcast(&pauseFactory);
            flag = true;
        }
        else if (currStored > lowerStore && currStored < upperStore && flag) {
            pthread_cond_broadcast(&pauseFactory);
        } 
        if (currStored >= upperStore) {
            flag = false;
        }
        usleep(5000);
        currStored = 0;
    }
    return NULL;
}

void* manufactureTypeA(void* data){
    int position = *((int*)data);
    int line = position/numOfWorkersA;
    int step = position%numOfWorkersA;
    usleep(500);
    int counter=0;
    while(true){
        if(currStored >= upperStore){
            pthread_cond_wait(&pauseFactory, &suspend_mutex);
            pthread_mutex_unlock(&suspend_mutex);
        } 
        if(step == 0){
            busyLineA[line][step] = true;
            Chocolate bar;
            bar.index = counter++;
            bar.type = 'A';
            bar.status = 0;
            bar.lock = false;
            int timeTaken = generate_Randoms(minA, maxA);
            sleep(timeTaken);
            doneLineA[line][0].push_back(bar);
            busyLineA[line][step] = false;
            usleep(50000);
        }
        else if(step>0 && step<=3){
            if(doneLineA[line][step-1].size()>0){
                busyLineA[line][step] = true;
                Chocolate bar = doneLineA[line][step-1][0];
                doneLineA[line][step-1].erase(doneLineA[line][step-1].begin());
                int timeTaken = generate_Randoms(minA, maxA);
                sleep(timeTaken);
                if(step==3)
                    randomA[line][bar.index] = bar;
                else
                    doneLineA[line][step].push_back(bar);
                busyLineA[line][step] = false;
                usleep(50000);
            }
        }
        else {
            int power = step - 4;
            int choices = pow(2, power + 1);
            usleep(500);
            pthread_mutex_lock(&lineA_mutex[line]);
            if (randomA[line].size() > 0) {
                map<int, Chocolate>::iterator bar;
                int barr;
                for (bar = randomA[line].begin(); bar != randomA[line].end() && !busyLineA[line][step]; bar++) {
                    if (bar->second.status % choices < pow(2, power) && !bar->second.lock) {
                        bar->second.lock = true;
                        barr = bar->first;
                        busyLineA[line][step] = true;
                    }
                }
                pthread_mutex_unlock(&lineA_mutex[line]);

                if (busyLineA[line][step]) {
                    int timeTaken = generate_Randoms(minA, maxA);
                    sleep(timeTaken);
                    randomA[line][barr].status += pow(2, power);

                    if (randomA[line][barr].status == 15) {
                        readyFromLines[0].push_back(randomA[line][barr]);
                        randomA[line].erase(barr);
                    } else {
                        randomA[line][barr].lock = false;
                    }
                    busyLineA[line][step] = false;
                    usleep(50000);
                }
            }
            else {
                pthread_mutex_unlock(&lineA_mutex[line]);
            }
        }
    }
}

void* manufactureTypeB(void* data){
    int position = *((int*)data);
    int line = position/numOfWorkersB;
    int step = position%numOfWorkersB;
    usleep(500);
    int counter =0;
    while(true){
        if(currStored >= upperStore){
            pthread_cond_wait(&pauseFactory, &suspend_mutex);
            pthread_mutex_unlock(&suspend_mutex);
        } 
        if(step == 0){
            busyLineB[line][step] = true;
            Chocolate bar;
            bar.index = counter++;
            bar.type = 'B';
            bar.status = 0;
            int timeTaken = generate_Randoms(minB, maxB);
            sleep(timeTaken);
            doneLineB[line][0].push_back(bar);
            busyLineB[line][step] = false;
            usleep(50000);
        }
        else{
            if(doneLineB[line][step-1].size()>0){
                busyLineB[line][step] = true;
                Chocolate bar = doneLineB[line][step-1][0];
                doneLineB[line][step-1].erase(doneLineB[line][step-1].begin());
                int timeTaken = generate_Randoms(minB, maxB);
                sleep(timeTaken);
                if(step==5)
                    readyFromLines[1].push_back(bar);
                else
                    doneLineB[line][step].push_back(bar);
                busyLineB[line][step] = false;
                usleep(50000);
            }
        }
        usleep(5000);
    }
}

void* manufactureTypeC(void* data){
    int position = *((int*)data);
    int line = position/numOfWorkersC;
    int step = position%numOfWorkersC;
    usleep(500);
    int counter=0;
    while(true){
        if(currStored >= upperStore){
            pthread_cond_wait(&pauseFactory, &suspend_mutex);
            pthread_mutex_unlock(&suspend_mutex);
        } 
        if(step == 0){
            busyLineC[line][step] = true;
            Chocolate bar;
            bar.index = counter++;
            bar.type = 'C';
            bar.status = 0;
            bar.lock = false;
            int timeTaken = generate_Randoms(minC, maxC);
            sleep(timeTaken);
            doneLineC[line][0].push_back(bar);
            busyLineC[line][step] = false;
            usleep(50000);
        }
        else if(step>0 && step<=2){
            if(doneLineC[line][step-1].size()>0){
                busyLineC[line][step] = true;
                Chocolate bar = doneLineC[line][step-1][0];
                doneLineC[line][step-1].erase(doneLineC[line][step-1].begin());
                int timeTaken = generate_Randoms(minC, maxC);
                sleep(timeTaken);
                if(step==2)
                    randomC[line][bar.index] = bar;
                else
                    doneLineC[line][step].push_back(bar);
                busyLineC[line][step] = false;
                usleep(50000);
            }
        }
        else {
            int power = step - 3;
            int choices = pow(2, power + 1);
            usleep(500);
            pthread_mutex_lock(&lineC_mutex[line]);
            if (randomC[line].size() > 0) {
                map<int, Chocolate>::iterator bar;
                int barr;
                for (bar = randomC[line].begin(); bar != randomC[line].end() && !busyLineC[line][step]; bar++) {
                    if (bar->second.status % choices < pow(2, power) && !bar->second.lock) {
                        bar->second.lock = true;
                        barr = bar->first;
                        busyLineC[line][step] = true;
                    }
                }
                pthread_mutex_unlock(&lineC_mutex[line]);

                if (busyLineC[line][step]) {
                    int timeTaken = generate_Randoms(minC, maxC);
                    sleep(timeTaken);
                    randomC[line][barr].status += pow(2, power);

                    if (randomC[line][barr].status == 3) {
                        readyFromLines[2].push_back(randomC[line][barr]);
                        randomC[line].erase(barr);
                    } else {
                        randomC[line][barr].lock = false;
                    }
                    busyLineC[line][step] = false;
                    usleep(50000);
                }
            }
            else {
                pthread_mutex_unlock(&lineC_mutex[line]);
            }
        }
    }
}

void *move_to_expiration_date(void* data) {
    int position;
    pthread_mutex_lock(&employee_mutex);
    position = empCounter;
    empCounter++;
    pthread_mutex_unlock(&employee_mutex);
    while (true) {
        bool found =false;
        pthread_mutex_lock(&ready_mutex);
        emplyeeState1[position] = 0;
        emplyeeDistance1[position] = 0;
        int turn = 0;
        for (int i = 0; i < 3 && !found; i++) {
            if (readyFromLines[i].size() >= 10) {
                found = true;
                turn = i;
            }
        }
        if(found){
            Chocolate batch [10];
            char type = readyFromLines[turn][0].type;
            for (int j = 0; j < 10 && readyFromLines[turn].size() > 0 ; j++) {
                //cout << "Printing expiration date on chocolate of type " << i << endl;
                batch[j] = readyFromLines[turn][0];
                readyFromLines[turn].erase(readyFromLines[turn].begin());
            }
            //cout << "Unlocked by " << position + 1 << endl;
            pthread_mutex_unlock(&ready_mutex);
            emplyeeState1[position]+= (1+turn);
            while(emplyeeState1[position] >=1 && emplyeeState1[position]<=3);
            for(int j=0 ; j<10 ; j++) {
                batches_to_print[turn].push_back(batch[j]);
            }
            emplyeeState1[position] = 5;
            while(emplyeeState1[position] == 5);
        }
        else{
            //cout << "Unlocked by " << position + 1 << endl;
            pthread_mutex_unlock(&ready_mutex);
        }

    }
    return NULL;
}

void *print_expiration_date(void *args){
    while(1){
        for(int i=0; i<3 ;i++){
            if(batches_to_print[i].size()>0){
                Chocolate batch = batches_to_print[i][0];
                usleep(expDateTime*50000);
                batches_to_print[i].pop_back();
                printed_batches[i].push_back(batch);
            }
        }
    }
}

void *separate_chocolates(void *arg) {
    int position;
    pthread_mutex_lock(&separator_employee_mutex);
    position = empCounter2;
    empCounter2++;
    pthread_mutex_unlock(&separator_employee_mutex);
    while (true) {
        emplyeeState2[position]=0;
        if(printed_batches[position].size()>0) {
            Chocolate batch = printed_batches[position][0];
            printed_batches[position].erase(printed_batches[position].begin());
            emplyeeState2[position]=1;
            while(emplyeeState2[position]==1);
            containers[position].push_back(batch);
            emplyeeState2[position]=3;
            while(emplyeeState2[position]==3);
        }
    }
    return NULL;
}

void* prepare_carton_boxes(void *arg) {
    int position;
    pthread_mutex_lock(&carton_employee_mutex);
    position = empCounter3;
    empCounter3++;
    pthread_mutex_unlock(&carton_employee_mutex);
    while (true) {
        vector<Chocolate> cartonV;
        emplyeeState3[position] = 0;
        if (containers[position].size() / 20 >= 1) {
            //cout << "possition " << position+1 << " got in as size is " << containers[position].size() << endl;
            for (int i = 0; i < 20 && containers[position].size() > 0 ; i++) {
                cartonV.push_back(containers[position][0]);
                containers[position].erase(containers[position].begin());
            }
            emplyeeState3[position] = 1;
            while (emplyeeState3[position] == 1);
            Box carton;
            carton.type = cartonV[0].type;
            for (int i = 0; i < 20 && cartonV.size() > 0 ; i++)
                cartonV.erase(cartonV.begin());
            carton_boxes[position].push_back(carton);
            emplyeeState3[position] = 3;
            while (emplyeeState3[position] == 3);
        }
    }
    return NULL;
}

void* collectToStorageArea(void* arg){
    int position = *(int*)arg;
    if(position >= 3 || position < 0){
        cout << "fatal error" << endl;
        return 0;
    }
    while(true){
        emplyeeState4[position] = 0;
        usleep(500);
        pthread_mutex_lock(&fillStorageEmployee_Mutex);
        int found = -1;
        Box carton;
        for(int j=0; j<3; j++){
            if(carton_boxes[j].size() > 0){
                found = j;
                carton = carton_boxes[found][carton_boxes[found].size() - 1];
                carton_boxes[found].pop_back();
                break;
            }
        }
        pthread_mutex_unlock(&fillStorageEmployee_Mutex);
        if(found!= -1) {
            emplyeeState4[position] = found+1;
            while(emplyeeState4[position]>=1 && emplyeeState4[position]<=3);
            storageArea[found]++;
            produced[found]++;
            sleep(storeTime);
            emplyeeState4[position] = 5;
            while(emplyeeState4[position]==5);
        }
        fillEmployeeAva++;
    }
}

void* collectToTruck(void* arg) {
    int position = *(int *) arg;
    while(true){
        pthread_mutex_lock(&sendToTruck_mutex);
        pthread_mutex_lock(&truck_mutex[position]);
        int found =-1;
        for(int i=0 ; i<3 ; i++){
            if(storageArea[i]>0 && currentTruck<3 &&
            truckBoxes[currentTruck][i] < truckSize[i] && !(truckBoxes[currentTruck][i] == truckSize[i]-1 && emplyeeState5[(position+1)%2]!=0)){
                found=i;
                storageArea[i]--;
                break;
            }
        }
        pthread_mutex_unlock(&sendToTruck_mutex);
        if(found > -1){
            emplyeeState5[position]=found+1;
            while(emplyeeState5[position]>=1 && emplyeeState5[position]<=3);
            while(emplyeeState5[position]>=4 && emplyeeState5[position]<=6);
            truckBoxes[currentTruck][found]++;
            pthread_mutex_unlock(&truck_mutex[position]);
            emplyeeState5[position]=8;
            while(emplyeeState5[position]==8);
            while(emplyeeState5[position]==9);
        }
        else
            pthread_mutex_unlock(&truck_mutex[position]);
    }
}

void* sendShipment(void* arg) {
    int position = *(int *) arg;
    while(1){
        truckState[position]=0;
        /*printf("Type A %d of %d, type B %d of %d, type C %d of %d\n", truckBoxes[position][0], truckSize[0],
               truckBoxes[position][1], truckSize[1], truckBoxes[position][2], truckSize[2]);*/
        if(truckBoxes[position][0]>=truckSize[0] && truckBoxes[position][1]>=truckSize[1] &&
        truckBoxes[position][2]>=truckSize[2]){
            system("canberra-gtk-play -f horn.ogg");
            truckState[position]=1;
            pthread_mutex_lock(&truck_mutex[0]);
            pthread_mutex_lock(&truck_mutex[1]);
            int next=1;
            for(; next<3 && truckState[(position+next)%3]!=0 ; next++);
            if(next<3) {
                currentTruck = (position + next) % 3;
            }
            else
                currentTruck=3;
            pthread_mutex_unlock(&truck_mutex[0]);
            pthread_mutex_unlock(&truck_mutex[1]);
            while(truckState[position]==1);
            sleep(shippingRate);
            for(int i=0 ; i<3 ; i++)
                truckBoxes[position][i]=0;
            truckState[position]=3;
            while(truckState[position]==3);
        }
    }
}

void* checkTrucks(void* arg){
    while(1){
        if(currentTruck==3){
            for(int i=0; i<3 ; i++){
                if(truckState[i]==0){
                    currentTruck=i;
                    break;
                }
            }
        }
    }
}

int generate_Randoms(int lower, int upper){
    srand(time(NULL) + seed);
    int num = (rand() % (upper - lower + 1)) + lower;
    seed++;
    return num ;
}

/********************************* Read File ****************************************************/
void readFile(){
    string format[] = { "MIN_MAX_A", "MIN_MAX_B", "MIN_MAX_C", "EXP_DATE_TIME", 
                        "TERMINATION_CASES", "STORE_TIME", "STORAGE_THRESH", "TRUCK_SIZE", 
                        "SHIPPING_TIME", "NUM_LINES_TYPE_A", "NUM_LINES_TYPE_B", "NUM_LINES_TYPE_C",
                    };
    string line;
    ifstream
            MyReadFile(fileName);
    while (getline(MyReadFile, line)) {
        stringstream ss(line);
        vector <string> vec;
        string temp;
        while (getline(ss, temp, ' ')) {
            vec.push_back(temp);
        }
        if (format[0].compare(vec[vec.size() - 1]) == 0) {
            maxA = stoi(vec[vec.size() - 2]);
            minA = stoi(vec[vec.size() - 3]);
        }
        else if (format[1].compare(vec[vec.size() - 1]) == 0) {
            maxB = stoi(vec[vec.size() - 2]);
            minB = stoi(vec[vec.size() - 3]);
        }
        else if (format[2].compare(vec[vec.size() - 1]) == 0) {
            maxC = stoi(vec[vec.size() - 2]);
            minC = stoi(vec[vec.size() - 3]);
        }
        else if (format[3].compare(vec[vec.size() - 1]) == 0) {
            expDateTime = stoi(vec[vec.size() - 2]);
        }
        else if (format[4].compare(vec[vec.size() - 1]) == 0) {
            targetA = stoi(vec[vec.size() - 5]);
            targetB = stoi(vec[vec.size() - 4]);
            targetC = stoi(vec[vec.size() - 3]);
            termTime = stoi(vec[vec.size() - 2]);
        }
        else if (format[5].compare(vec[vec.size() - 1]) == 0) {
            storeTime = stoi(vec[vec.size() - 2]);
        }
        else if (format[6].compare(vec[vec.size() - 1]) == 0) {
            upperStore = stoi(vec[vec.size() - 2]);
            lowerStore = stoi(vec[vec.size() - 3]);
        }
        else if (format[7].compare(vec[vec.size() - 1]) == 0) {
            truckSize[2] = stoi(vec[vec.size() - 2]);
            truckSize[1] = stoi(vec[vec.size() - 3]);
            truckSize[0] = stoi(vec[vec.size() - 4]);
        }
        else if (format[8].compare(vec[vec.size() - 1]) == 0) {
            shippingRate = stoi(vec[vec.size() - 2]);
        }
        else if (format[9].compare(vec[vec.size() - 1]) == 0) {
            numOfLinesTypeA = stoi(vec[vec.size() - 2]);
        }
        else if (format[10].compare(vec[vec.size() - 1]) == 0) {
            numOfLinesTypeB = stoi(vec[vec.size() - 2]);
        }
        else if (format[11].compare(vec[vec.size() - 1]) == 0) {
            numOfLinesTypeC = stoi(vec[vec.size() - 2]);
        }
        else {
            perror("file error");
            exit(1);
        }
    }
}

/********************************* openGL *********************************************/

void startOpenGl(){
    init();
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1220, 1178);
    glutInitWindowPosition(200, 100);
    glutCreateWindow("Chocolate Factory");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timer, 0);
    init();
    glutMainLoop();
}

void renderBitMap(double x, double y, void *font, char *string) {
    char *c;
    glRasterPos2f(x, y);
    for (c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    /// Red Edges
    rectangle(-2, 69, 40, 67, 1, 0, 0);
    rectangle(42, 69, 40, -15, 1, 0, 0);
    rectangle(-2, 67, 0, 103, 1, 0, 0);
    rectangle(-2, 101, 125, 103, 1, 0, 0);
    rectangle(123, -15, 125, 103, 1, 0, 0);
    rectangle(40, -15, 125, -13, 1, 0, 0);
    rectangle(40, 30, 125, 28, 1, 0, 0);
    /// Left Lines
    for(int i=0; i<4 ; i++) {
        glBegin(GL_LINES);
        glColor3f(0,0,0);
        glVertex2f(0, 100 - (10*i));
        glVertex2f(40, 100 - (10*i));
        glEnd();
    }
    glBegin(GL_LINES);
    glColor3f(0,0,0);
    glVertex2f(0, 100);
    glVertex2f(0, 70);
    glEnd();
    for(int i=0; i<4 ; i++) {
        int w;
        if(i!=0)
            w=1;
        glBegin(GL_LINES);
        glColor3f(0,0,0);
        glVertex2f(13*i + w, 100);
        glVertex2f(13*i + w, 70);
        glEnd();
        if(i>=2){
            glBegin(GL_LINES);
            glColor3f(1,1,1);
            glVertex2f(13*i + w, 80);
            glVertex2f(13*i + w, 90);
            glEnd();
        }
    }
    /// Lines manufacture workers
    for(int i=0 ; i<3; i++){
        rectangle(2, 100-(i*10)-4, 12, 100-(i*10)-6, 1, 0, 1);
        for(int j=0; j<8 ; j++){
            int row=j/4;
            int col = j%4;
            int busy = 0;
            if(busyLineA[i][j])
                busy = 1;
            rectangle(2+(col*3), 100-(10*i)-2-(5*row), 3+(col*3), 100-(10*i)-3-(5*row), busy, 0, 0);
        }
    }
    for(int i=0; i<2;i++){
        for(int j=0; j<2 ; j++){
            rectangle(16+(13*j), 100-(i*20)-4, 25+(13*j), 100-(i*20)-6, 0, i%2, 1);
            for(int k=0 ; k<6 ; k++){
                if(i!=1 || k!=5) {
                    int row=k/3;
                    int col = k%3;
                    if( (i==0 && busyLineB[j][k]) || (i==1 && busyLineC[j][k]) )
                        rectangle(18+(13*j)+(col*3), 100-(20*i)-2-(5*row), 17+(13*j)+(col*3), 100-(20*i)-3-(5*row), 1, 0, 0);
                    else
                        rectangle(18+(13*j)+(col*3), 100-(20*i)-2-(5*row), 17+(13*j)+(col*3), 100-(20*i)-3-(5*row), 0, 0, 0);
                }
            }
        }
    }
    /// Employees who move to printer
    for(int i=0; i<2; i++) {
        x1[i] = 39 + emplyeeDistance1[i];
        if(emplyeeState1[i]==1)
            rectangle(x1[i], 90-(5*i)-2, x1[i]+1, 90-(5*i)-3, 1, 0, 1);
        else if(emplyeeState1[i]==2)
            rectangle(x1[i], 90-(5*i)-2, x1[i]+1, 90-(5*i)-3, 0, 0, 1);
        else if(emplyeeState1[i]==3)
            rectangle(x1[i], 90-(5*i)-2, x1[i]+1, 90-(5*i)-3, 0, 1, 1);
        else
            rectangle(x1[i], 90-(5*i)-2, x1[i]+1, 90-(5*i)-3, 0, 0, 0);
    }
    //glFlush();
    for(int i=0; i<2 ; i++){
        glBegin(GL_LINES);
        glColor3f(0,0,0);
        glVertex2f(123 - (45*i) , 95);
        glVertex2f(123 - (45*i), 65);
        glEnd();
        glBegin(GL_LINES);
        glColor3f(0,0,0);
        glVertex2f(78 , 95 - (30*i));
        glVertex2f(123 , 95 - (30*i));
        glEnd();
    }
    rectangle(80, 67, 121, 93, 0, 1, 0);

    /// Trucks
    for(int i=0; i<3; i++){
        for(int j=0; j<2 ; j++){
            rectangle(12+(15*j) - truckDistance[i], -10+(27*i), 17+(15*j) - truckDistance[i], 10+(27*i), 0, 0, 0);
        }
        rectangle(10 - truckDistance[i], -8+(27*i), 35 - truckDistance[i], 8+(27*i), 1, 1, 0);
        rectangle(3 - truckDistance[i], -9+(27*i), 10 - truckDistance[i], 9+(27*i), 0, 1, 0);
    }

    rectangle(42, 99, 43, 100, 1, 0, 1);
    rectangle(52, 99, 53, 100, 0, 0, 1);
    rectangle(62, 99, 63, 100, 0, 1, 1);

    rectangle(16, 88, 17, 89, 1, 0, 1);
    rectangle(16, 85, 17, 86, 0, 0, 1);
    rectangle(16, 82, 17, 83, 0, 1, 1);

    rectangle(81, 80, 84, 83, 1, 0, 1);
    rectangle(81, 75, 84, 78, 0, 0, 1);
    rectangle(81, 70, 84, 73, 0, 1, 1);

    for(int i=0 ; i<3 ; i++){
        rectangle(80 + (17*i), 35, 87 + (17*i), 55, 1, 1, 0);
    }

    for(int i=0 ; i<2 ; i++) {
        rectangle(81, 54 - (13*i), 86, 49 - (13*i), 1, 0, 1);
        rectangle(98, 54 - (13*i), 103, 49 - (13*i), 0, 0, 1);
        rectangle(115, 54 - (13*i), 120, 49 - (13*i), 0, 1, 1);
    }

    for(int i=0 ; i<3 ; i++){
        rectangle(83 + (17*i), 63 - emplyeeDistance2[i], 84 + (17*i), 64 - emplyeeDistance2[i], 0, 0, 0);
        rectangle(83 + (17*i), 49 - emplyeeDistance3[i], 84 + (17*i), 48 - emplyeeDistance3[i], 0, 0, 0);
    }

    /// Warehouse
    rectangle(58, -4, 66, 4, 1, 0, 1);
    rectangle(76, -4, 84, 4, 0, 0, 1);
    rectangle(94, -4, 102, 4, 0, 1, 1);
    glBegin(GL_LINES);
    glColor3f(1,0,0);
    glVertex2f(42, 5);
    glVertex2f(123, 5);
    glEnd();

    /// Moving to Warehouse Employees
    for(int i=0 ; i<2 ; i++){
        switch(emplyeeState4[i]){
            case 1: rectangle(92 + (17*i), 33-emplyeeDistance4[i], 93 + (17*i), 32-emplyeeDistance4[i], 1, 0, 1);
                break;
            case 2: rectangle(92 + (17*i), 33-emplyeeDistance4[i], 93 + (17*i), 32-emplyeeDistance4[i], 0, 0, 1);
                break;
            case 3: rectangle(92 + (17*i), 33-emplyeeDistance4[i], 93 + (17*i), 32-emplyeeDistance4[i], 0, 1, 1);
                break;
            case 4: rectangle(92 + (17*i), 33-emplyeeDistance4[i], 93 + (17*i), 32-emplyeeDistance4[i], 1, 0, 0);
                break;
            default: rectangle(92 + (17*i), 33-emplyeeDistance4[i], 93 + (17*i), 32-emplyeeDistance4[i], 0, 0, 0);
        }
    }
    for(int i=0 ; i<2 ; i++){
        if(emplyeeState5[i]>=1 && emplyeeState5[i]<=6) {
            if(emplyeeState5[i]%3==1)
                rectangle(50 - emplyeeDistance5x[i], -5 + (4 * i) + emplyeeDistance5y[i], 51 - emplyeeDistance5x[i],
                      -4 + (4 * i) + emplyeeDistance5y[i], 1, 0, 1);
            else if(emplyeeState5[i]%3==2)
                rectangle(50 - emplyeeDistance5x[i], -5 + (4 * i) + emplyeeDistance5y[i], 51 - emplyeeDistance5x[i],
                          -4 + (4 * i) + emplyeeDistance5y[i], 0, 0, 1);
            else
                rectangle(50 - emplyeeDistance5x[i], -5 + (4 * i) + emplyeeDistance5y[i], 51 - emplyeeDistance5x[i],
                          -4 + (4 * i) + emplyeeDistance5y[i], 0, 1, 1);
        }
        else
            rectangle(50 - emplyeeDistance5x[i], -5 + (4 * i) + emplyeeDistance5y[i], 51 - emplyeeDistance5x[i],
                      -4 + (4 * i) + emplyeeDistance5y[i], 0, 0, 0);
    }

    for(int i=0 ; i<3 ; i++){
        rectangle(16 - truckDistance[i], -5 + (i*27), 19 - truckDistance[i], -2 + (i*27), 1, 0, 1);
        rectangle(21 - truckDistance[i], -5 + (i*27), 24 - truckDistance[i], -2 + (i*27), 0, 0, 1);
        rectangle(26 - truckDistance[i], -5 + (i*27), 29 - truckDistance[i], -2 + (i*27), 0, 1, 1);
    }

    for(int i=0 ; i<16 ; i++){
        rectangle(44 + (3*i), 20, 43+(3*i), 21, 1, 0, 1);
        rectangle(44 + (3*i), 16, 43+(3*i), 17, 0, 0, 1);
        rectangle(44 + (3*i), 12, 43+(3*i), 13, 0, 1, 1);
        rectangle(44 + (3*i), 8, 43+(3*i), 9, 1, 0, 1);
    }

    for(int i=0 ; i<4 ; i++){
        rectangle(97 + (3*i), 20, 96+(3*i), 21, 1, 0, 1);
        rectangle(97 + (3*i), 16, 96+(3*i), 17, 0, 0, 1);
        rectangle(97 + (3*i), 12, 96+(3*i), 13, 0, 1, 1);
        rectangle(97 + (3*i), 8, 96+(3*i), 9, 1, 0, 1);
    }

    for(int i=0; i<3 ; i++){
        rectangle(113 + (3*i), 20, 114+(3*i), 21, 1, 0, 1);
        rectangle(113 + (3*i), 16, 114+(3*i), 17, 0, 0, 1);
        rectangle(113 + (3*i), 12, 114+(3*i), 13, 0, 1, 1);
        rectangle(113 + (3*i), 8, 114+(3*i), 9, 1, 0, 1);
    }

    displayPlacesNames();
    glFlush();
    glutSwapBuffers();
}

void DatePrinterDisplay(){
    char *info = (char*)"Date Printer";
    glColor3f(0, 0, 0);
    renderBitMap(81, 90, GLUT_BITMAP_TIMES_ROMAN_24, info);
    char *ready1 = (char *) malloc(sizeof(long long));
    sprintf(ready1, "Units in Printer: %ld", batches_to_print[0].size());
    renderBitMap(85, 80, GLUT_BITMAP_TIMES_ROMAN_24, ready1);
    char *ready2 = (char *) malloc(sizeof(long long));
    sprintf(ready2, "Units in Printer: %ld", batches_to_print[1].size());
    renderBitMap(85, 75, GLUT_BITMAP_TIMES_ROMAN_24, ready2);
    char *ready3 = (char *) malloc(sizeof(long long));
    sprintf(ready3, "Units in Printer: %ld", batches_to_print[2].size());
    renderBitMap(85, 70, GLUT_BITMAP_TIMES_ROMAN_24, ready3);
}

void ContainerDisplay(){
    glColor3f(0, 0, 0);
    for(int i=0 ; i<3 ; i++) {
        char *ready1 = (char *) malloc(sizeof(long long));
        sprintf(ready1, "%ld", containers[i].size());
        renderBitMap(82 + (17*i), 50, GLUT_BITMAP_TIMES_ROMAN_24, ready1);
        char *ready2 = (char *) malloc(sizeof(long long));
        sprintf(ready2, "%ld", carton_boxes[i].size());
        renderBitMap(82 + (17*i), 37, GLUT_BITMAP_TIMES_ROMAN_24, ready2);
    }

    for(int i=0 ; i<2 ; i++){
        char *type1 = (char*)"Containers";
        renderBitMap(89 + (17*i), 50, GLUT_BITMAP_TIMES_ROMAN_10, type1);
        char *type2 = (char*)"Boxes";
        renderBitMap(90 + (17*i), 38, GLUT_BITMAP_TIMES_ROMAN_10, type2);
    }
}

void warehouseDisplay(){
    glColor3f(0, 0, 0);
    char *info = (char*)"Warehouse";
    renderBitMap( 44, 24, GLUT_BITMAP_TIMES_ROMAN_24, info);
    glColor3f(0, 0, 0);
    for(int i=0 ; i<3 ; i++) {
        char *ready = (char *) malloc(sizeof(long long));
        sprintf(ready, "%d", storageArea[i]);
        renderBitMap(60 + (18*i), -2, GLUT_BITMAP_TIMES_ROMAN_24, ready);
    }
}

void truckDisplay(){
    glColor3f(0, 0, 0);
    for(int i=0 ; i<3 ; i++){
        char *info = (char *) malloc(sizeof(long long));
        sprintf(info, "Truck %d", i+1);
        renderBitMap(17 - truckDistance[i], 1 +(27*i), GLUT_BITMAP_TIMES_ROMAN_24, info);
        for(int j=0 ; j<3 ; j++){
            char *ready = (char *) malloc(sizeof(long long));
            sprintf(ready, "%d", truckBoxes[i][j]);
            renderBitMap(17 +(5*j) - truckDistance[i], -4 + (27*i), GLUT_BITMAP_TIMES_ROMAN_10, ready);
        }
    }

}

void displayPlacesNames(){
    ContainerDisplay();
    DatePrinterDisplay();
    warehouseDisplay();
    truckDisplay();
    char *type1 = (char*)"Type A";
    renderBitMap(44, 99, GLUT_BITMAP_TIMES_ROMAN_10, type1);
    char *type2 = (char*)"Type B";
    renderBitMap(54, 99, GLUT_BITMAP_TIMES_ROMAN_10, type2);
    char *type3 = (char*)"Type C";
    renderBitMap(64, 99, GLUT_BITMAP_TIMES_ROMAN_10, type3);
    char *ready1_1 = (char *) malloc(sizeof(long long));
    sprintf(ready1_1, "Ready: %ld", readyFromLines[0].size());
    renderBitMap(18, 88, GLUT_BITMAP_TIMES_ROMAN_10, ready1_1);
    char *ready1_2 = (char *) malloc(sizeof(long long));
    sprintf(ready1_2, "Ready: %ld", readyFromLines[1].size());
    renderBitMap(18, 85, GLUT_BITMAP_TIMES_ROMAN_10, ready1_2);
    char *ready1_3 = (char *) malloc(sizeof(long long));
    sprintf(ready1_3, "Ready: %ld", readyFromLines[2].size());
    renderBitMap(18, 82, GLUT_BITMAP_TIMES_ROMAN_10, ready1_3);
}

void reshape(int w, int h){
    glViewport(0,0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-2,125,-15,103);
    glMatrixMode(GL_MODELVIEW);
}

void timer(int){
    glutPostRedisplay();
    glutTimerFunc(1000/60,timer,0);
    for(int i=0 ; i<2 ; i++){
        if (emplyeeState1[i]>=1 && emplyeeState1[i] <=3){
            if(emplyeeDistance1[i] < 38)
                emplyeeDistance1[i]++;
            else
                emplyeeState1[i] = 4;
        }
        else if(emplyeeState1[i]==5){
            if(emplyeeDistance1[i] > 0)
                emplyeeDistance1[i]--;
            else
                emplyeeState1[i] = 0;
        }

        if(emplyeeState4[i]>=1 && emplyeeState4[i]<=3){
            if(emplyeeDistance4[i] < 27)
                emplyeeDistance4[i]+=1;
            else
                emplyeeState4[i] = 4;
        }
        else if(emplyeeState4[i]==5){
            if(emplyeeDistance4[i] > 0)
                emplyeeDistance4[i]-=1;
            else
                emplyeeState4[i] = 0;
        }

        if(emplyeeState5[i]>=1 && emplyeeState5[i]<=3){
            if(emplyeeDistance5x[i]<12)
                emplyeeDistance5x[i]++;
            else
                emplyeeState5[i]+=3;
        }
        else if(emplyeeState5[i]>=4 && emplyeeState5[i]<=6){
            if(emplyeeDistance5y[i]<truckLevel[currentTruck])
                emplyeeDistance5y[i]++;
            else
                emplyeeState5[i]=7;
        }
        else if(emplyeeState5[i]==8){
            if(emplyeeDistance5y[i]>0)
                emplyeeDistance5y[i]--;
            else
                emplyeeState5[i]=9;
        }
        else if(emplyeeState5[i]==9){
            if(emplyeeDistance5x[i]>0)
                emplyeeDistance5x[i]--;
            else
                emplyeeState5[i]=0;
        }
    }

    for(int i=0 ; i<3 ; i++){
        if(emplyeeState2[i]==1){
            if(emplyeeDistance2[i] < 10)
                emplyeeDistance2[i]+=1;
            else
                emplyeeState2[i] = 2;
        }
        else if(emplyeeState2[i]==3){
            if(emplyeeDistance2[i] > 0)
                emplyeeDistance2[i]-=1;
            else
                emplyeeState2[i] = 0;
        }

        if(emplyeeState3[i]==1){
            if(emplyeeDistance3[i] < 7)
                emplyeeDistance3[i]++;
            else
                emplyeeState3[i] = 2;
        }
        else if(emplyeeState3[i]==3){
            if(emplyeeDistance3[i] > 0)
                emplyeeDistance3[i]--;
            else
                emplyeeState3[i] = 0;
        }

        if(truckState[i]==1){
            if(truckDistance[i] < 40)
                truckDistance[i]++;
            else
                truckState[i]=2;
        }
        else if(truckState[i]==3){
            if(truckDistance[i] > 0)
                truckDistance[i]--;
            else
                truckState[i]=0;
        }
    }
}

void rectangle(int x1, int y1, int x2, int y2, int r, int g, int b){
    glBegin(GL_POLYGON);
    glColor3f(r, g, b);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}