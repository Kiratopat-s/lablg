#include <lgpio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int key = 25;
int count = 0;
int h;

void isrFunc(int gpio,lgGpioAlert_p evt,void *data);

int main(){
    int e;
    if((h=lgGpiochipOpen(0)) < 0) exit(-1);

    printf("Set G%d as input\n",key);
    // Set callback function
    lgGpioSetAlertsFunc(h,key,isrFunc,NULL);

    e=lgGpioClaimAlert(h,LG_SET_PULL_UP,LG_RISING_EDGE,key,-1);

    while(count<10){
        printf("Key pressed : %2d\r",count);
        fflush(stdout);
        usleep(250000);
    }

    lgGpiochipClose(h);
    return 0;
}

void isrFunc(int gpio,lgGpioAlert_p evt,void *data){
   count++;
}



