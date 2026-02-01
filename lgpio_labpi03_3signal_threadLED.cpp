#include <lgpio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

int ledGPIO[4]={4,5,6,12};
int ledLevel[4]={0};
int h;
int stop = false;
void initGPIO();

void *blinkingLED(void *param);

void gpio_stop(int sig){
    printf("User pressing CTRL-C\n");
	kill(getpid(),SIGUSR1);
}

void thread_stop(int sig){
    printf("Recieving USR signal\n");
    stop = true;
}

int main(){
    pthread_t tid;
    pthread_attr_t attr;
    int i;

    initGPIO();

    pthread_attr_init(&attr);
    pthread_create(&tid,&attr,blinkingLED,NULL);

    printf("Waiting all threads to stop...\n");
    pthread_join(tid,NULL);
    pthread_attr_destroy(&attr);

	lgGpiochipClose(h);
    return 0;
}

void initGPIO(){
    int i;
    if((h=lgGpiochipOpen(0)) < 0) exit(-1);
    signal(SIGINT,gpio_stop);
	signal(SIGUSR1,thread_stop);
    lgGroupClaimOutput(h,0,4,ledGPIO,ledLevel);
}

void *blinkingLED(void *param){
    int i,j;
    int pattern[2][4]={{1,0,1,0},{0,1,0,1}};

    while(1){
        for(i=0;i<2;i++){
            for(j=0;j<4;j++)
                lgGpioWrite(h,ledGPIO[j],pattern[i][j]);
			usleep(250000);
        }
        if(stop) break;
    }
    for(j=0;j<4;j++)
        lgGpioWrite(h,ledGPIO[i],0);
    pthread_exit(0);
}

