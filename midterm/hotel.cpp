#include "local.h"

void* receptionJob(void* arg);
void* bellboyJob(void* arg);

pthread_mutex_t
reception_mutex = PTHREAD_MUTEX_INITIALIZER,
looking_for_room_mutex = PTHREAD_MUTEX_INITIALIZER,
bellboy_mutex = PTHREAD_MUTEX_INITIALIZER;

int RECEPTION_EMPLOYEES =5, BELLBOY_EMPLOYEES=10,
        NUMBER_OF_BEDROOMS_BY_SPACE[5] = {20, 30, 20, 20, 10}, PRICES_OF_ROOMS_FOR_ONE_NIGHT[5] = {200, 300, 400, 500, 600},
        TIP_MIN = 10, TIP_MAX = 30, BELLBOY_PSYCHOLOGICAL_STATE_MIN = 3, BELLBOY_PSYCHOLOGICAL_STATE_MAX =9, PSYCHOLOGICAL_STATE_IN_ORDER_TO_GET_TIPPED =7,
        SENDING_LUGGAGE_TIME_MIN = 20, SENDING_LUGGAGE_TIME_MAX = 30;

vector<int>pre_booked[5];
vector<TOURIST_GROUP> tourist_on_reception;
vector<TOURIST_GROUP> waiting[5];
vector<ROOM>available_rooms[5];
vector<LUGGAGE>available_luggage;
int profit =0;


int main() {
    pthread_t reception[RECEPTION_EMPLOYEES];
    pthread_t bellboy[BELLBOY_EMPLOYEES];

    for (int i = 0; i < RECEPTION_EMPLOYEES; i++) {
        pthread_create(&reception[i], nullptr, receptionJob, (void *) &i);
        usleep(50000);
    }

    for(int i=0 ; i<BELLBOY_EMPLOYEES ; i++){
        pthread_create(&bellboy[i], nullptr, bellboyJob, (void *) &i);
        usleep(50000);
    }

    return 0;
}

void* receptionJob(void* arg){
    int index = *(int *) arg;
    TOURIST_GROUP dealingWith;
    bool found = false;
    while(true){
        pthread_mutex_lock(&reception_mutex);
        if(tourist_on_reception.size()>0){
            found = true;
            dealingWith = tourist_on_reception[0];
            tourist_on_reception.erase(tourist_on_reception.begin());
        }
        pthread_mutex_unlock(&reception_mutex);
        if(found){
            int touristSize = dealingWith.people;
            if(touristSize>=5)
                touristSize = 5;
            touristSize--;
            bool found2 = false;
            pthread_mutex_lock(&looking_for_room_mutex);
            ROOM given_Room;
            if(available_rooms[touristSize].size() > 0){
                given_Room = available_rooms[touristSize][0];
                found2 = true;
            }
            pthread_mutex_unlock(&looking_for_room_mutex);
            if(found2){
                dealingWith.room = given_Room;
                profit+= PRICES_OF_ROOMS_FOR_ONE_NIGHT[touristSize];
            }
            else{
                waiting[touristSize].push_back(dealingWith);
            }
        }
    }
    return NULL;
}

void* bellboyJob(void* arg){
    int index = *(int *) arg;
    LUGGAGE carrying[2];
    while(true){
        bool found =false;
        pthread_mutex_lock(&bellboy_mutex);
        if(available_luggage.size()>=2){
            carrying[0] = available_luggage[0];
            carrying[1] = available_luggage[1];
            found = true;
        }
        pthread_mutex_unlock(&bellboy_mutex);
        if(found){

        }
    }
    return NULL;
}