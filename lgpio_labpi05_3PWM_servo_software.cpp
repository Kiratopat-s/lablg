#include <lgpio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include "pi.h"

int pwm=50;
int PWM_pin=18;
int running=true;
int h;

void gpio_stop(int sig);
void *servoPWM(void *param);

int main(){
    pthread_t tid;
    pthread_attr_t attr;
    int i;

    if((h=lgGpiochipOpen(0)) < 0) exit(-1);
    signal(SIGINT,gpio_stop);

    lgGpioClaimOutput(h,0,PWM_pin,0);
    pthread_attr_init(&attr);
    pthread_create(&tid,&attr,servoPWM,NULL);

    while(running){
        for(i=0;i<100;i++){
            pwm=i;
            if(!running)break;
            usleep(20000);
        }
        for(i=100;i>0;i--){
            pwm=i;
            if(!running)break;
            usleep(20000);
        }
    }

    pthread_join(tid,NULL);
    pthread_attr_destroy(&attr);
    lgGpiochipClose(h);
    return 0;
}

void *servoPWM(void *param){
    int pON,pOFF;
    //Increase thread priority to keep
    // the precision or pulse width (pON)
    piHiPri(50);
    while(running){
        pON = pwm*10+1000;
        pOFF = (100-pwm)*10+18000;
        lgGpioWrite(h,PWM_pin,1);
        udelay(pON);
        lgGpioWrite(h,PWM_pin,0);
        yield();
        usleep(pOFF);
    }
    piHiPri(0);
    pthread_exit(0);
}

void gpio_stop(int sig){
    printf("User pressing CTRL-C");
    running=false;
}

