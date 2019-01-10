//
//  main.c
//  Mutual Exclusion and Synchronization Train Automation
//
//  Created by Khaled Mohab on 12/6/18.
//  Copyright © 2018 Khaled Mohab. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
//Mutex Variables.
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int threadsComplete;
bool trainReady;

//station struct
struct station {
    pthread_mutex_t mutexTrain;
    pthread_cond_t isTrainInStation;
    pthread_cond_t isPassengerSeated;
    int numberOfPeopleInStation;
    int standingPassengerInTrain;
    int trainNumberOfSeatsAvailable;
};

int maxStation=100;
int maxTrain=60;
    //single lock in each struct station.
//station_init
void station_init(struct station * station)
{
    station->numberOfPeopleInStation=0;
    station->trainNumberOfSeatsAvailable=0;
    station->standingPassengerInTrain=0;
}
//station_wait_for_train(struct station *station).

void station_wait_for_train(struct station *station)
{
    pthread_mutex_lock(&station->mutexTrain);
    if(station->numberOfPeopleInStation < maxStation) {
        station->numberOfPeopleInStation++;
        pthread_cond_wait(&station->isTrainInStation, &station->mutexTrain);
    }
    //station->numberOfPeopleInStation++;
    //station->standingPassengerInTrain++;
    pthread_mutex_unlock(&station->mutexTrain);

}
//thread for passenger
void *passenger(void * arg)
{
    struct station *station = (struct station*)arg;
    station_wait_for_train(station);
    __sync_add_and_fetch(&threadsComplete, 1);
    
    return NULL;
}
struct trainThread
{
    struct station *station;
    int count;
};
//thread for train

//station_load_train(struct station *station, int count)
void station_load_train(struct station *station, int count)
{
    pthread_mutex_lock(&station->mutexTrain);
    //see how much space available on train
    station->trainNumberOfSeatsAvailable=count;
    //while loop for space on train or number of passengers waiting is 0
    printf("number of passengers in station is %d \n",station->numberOfPeopleInStation);

    while (station->numberOfPeopleInStation>0 && station->trainNumberOfSeatsAvailable>0) {
        pthread_cond_broadcast(&station->isTrainInStation);
        pthread_cond_wait(&station->isPassengerSeated, &station->mutexTrain);
        station->numberOfPeopleInStation--;
        station->standingPassengerInTrain++;
    }
    printf("number of passengers on train is %d \n",station->standingPassengerInTrain);
    printf("number of available seats is %d \n",station->trainNumberOfSeatsAvailable);


    pthread_mutex_unlock(&station->mutexTrain);

}

//station_on_board(struct station *station)
void station_on_board(struct station *station)
{
    pthread_mutex_lock(&station->mutexTrain);
     //let the train knows that it is on board
    
    while ((station->standingPassengerInTrain!=0)&&(station->trainNumberOfSeatsAvailable!=0))
    {
        pthread_cond_signal(&station->isPassengerSeated);
        station->standingPassengerInTrain--;
        station->trainNumberOfSeatsAvailable--;
        __sync_sub_and_fetch(&threadsComplete,1);
    }
    station->numberOfPeopleInStation+=station->standingPassengerInTrain;
    station->standingPassengerInTrain=0;
    pthread_mutex_unlock(&station->mutexTrain);

}
void* train(void * arg)
{
    struct trainThread * train=(struct trainThread*)arg;
    printf("train is comming with %d empty seats\n",train->count);
    station_load_train(train->station, train->count);
    station_on_board(train->station);
    trainReady=true;
    pthread_exit(NULL);
}
int main(int argc, const char * argv[]) {
    clock_t begin;
    struct station * station=malloc(sizeof(struct station));
    struct trainThread * trainA=malloc(sizeof(struct trainThread));
    station_init(station);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    do
    {
        begin = clock();
        while ( (double)(clock() - begin) / CLOCKS_PER_SEC < 0.001)
        {
            if (rand()%2)
            {
                pthread_t tid;
                pthread_create(&tid, &attr, passenger, station);
            }
            
        }
        printf("attention %d passengers the train is entering the station\n",station->numberOfPeopleInStation);
        printf("train is comming what is the number of empty seats\n");
        scanf("%d",&trainA->count);
        trainA->station=station;
        trainReady=false;
        //trainA->count=10;
        pthread_t tid;
        pthread_create(&tid, &attr, train, trainA);
        //station_load_train(station, 8);
        pthread_join(tid, NULL);
    }while (station->numberOfPeopleInStation!=0);
    return 0;
}
