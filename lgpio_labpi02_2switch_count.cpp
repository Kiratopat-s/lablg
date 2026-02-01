#include <lgpio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

int key = 25;
int count = 0;
int h;

void *getKey(void *param);
void *displayKey(void *param);

int main(){
    int i;
	pthread_t tid[2];
    pthread_attr_t attr[2];
    void *(*thread[2])(void *)={getKey,displayKey};
    if((h=lgGpiochipOpen(0)) < 0) exit(-1);

    //Set as input with pull-up resistor enabled
    lgGpioClaimInput(h,LG_SET_PULL_UP,key);
    printf("Set G%d as input\n",key);

    for(i=0;i<2;i++){
        pthread_attr_init(&attr[i]);
        pthread_create(&tid[i],&attr[i],thread[i],NULL);
    }
    for(i=0;i<2;i++){
        pthread_join(tid[i],NULL);
    }

    for(i=0;i<2;i++){
        pthread_attr_destroy(&attr[i]);
    }

    lgGpiochipClose(h);
    return 0;
}

void *getKey(void *param){
    while(count<10){
		while(lgGpioRead(h,key))
			usleep(100000); // wait for key press
		while(!lgGpioRead(h,key))
			usleep(100000); // wait for relasing key
		count++;
	}
	pthread_exit(NULL);
}
void *displayKey(void *param){
	while(count<10){
		printf("Key pressed : %2d\r",count);
		fflush(stdout);
		usleep(250000);
	}
	pthread_exit(NULL);
}





