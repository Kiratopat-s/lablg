#include <lgpio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#define MAXTIMEOUT 100000
#define MAXDISTANCE 20000
int trig=19;
int echo=20;
int h;

void initGPIO();
int ultraSonic();
int average(int data[],int count);
double toCm(int distance);

void gpio_stop(int sig){
    printf("User pressing CTRL-C");
    lgGpiochipClose(h);
    exit(0);
}


int main(){
    int distance[8]={0};
    int delay;
    int count=0;

    initGPIO();
    signal(SIGINT,gpio_stop);
    printf("Press CTRL-C to stop\n\n");
    while(1){
        delay = ultraSonic();
        if(delay<0){
            usleep(100000);
            continue;
        }
        if(delay>MAXDISTANCE) delay = MAXDISTANCE;
        distance[count]= delay;
        count = (count+1)%8;
        delay = average(distance,8);
        printf("Distance = %5d  (%5.1fcm)     \r",
            delay,toCm(delay));
        fflush(stdout);
        usleep(100000);
    }
    lgGpiochipClose(h);
    return 0;
}

void initGPIO(){
    if((h=lgGpiochipOpen(0)) < 0) exit(-1);

    //Set trigger pin as output
    lgGpioClaimOutput(h,0,trig,0);

    //Set echo pin as input
    lgGpioClaimInput(h,LG_SET_PULL_NONE,echo);
    sleep(2);
}

int ultraSonic(){
    int timeout;
    struct timespec start_time,end_time;
    double usec;

    lgGpioWrite(h,trig,1);
    usleep(10);
    lgGpioWrite(h,trig,0);
    for(timeout=0;(timeout<MAXTIMEOUT)&&(lgGpioRead(h,echo)==0);timeout++)usleep(10);
    clock_gettime(CLOCK_MONOTONIC,&start_time);
    if(timeout>=MAXTIMEOUT)return -1;
    for(timeout=0;(timeout<MAXTIMEOUT)&&(lgGpioRead(h,echo)==1);timeout++)usleep(10);
    clock_gettime(CLOCK_MONOTONIC,&end_time);
    if(timeout>=MAXTIMEOUT)return -1;
    usec = (double)(end_time.tv_sec - start_time.tv_sec)*1000000
         + (double)(end_time.tv_nsec - start_time.tv_nsec)/1000;
    return (int)usec;
}

int average(int data[],int count){
    int total=0,sum=0;
    for(int i=0;i<count;i++){
        if((data[i]>0)&&(data[i]<=MAXDISTANCE)){
            sum += data[i];
            total++;
        }
    }
    // If there is any valid data
    if(total) 
        return sum/total;
    // No valid data
    return -1;
}

double toCm(int distance){
    double sec = (double)distance/1000000;
    // raw distance is in microseconds
    // Speed of sound in the air = 331m/s
    // Sec = time spent from sound traveling from sensor
    // to the object and back to the sensor
    // 100 = adjust the unit from metre to centimetre
    return sec*331*100/2;
}
