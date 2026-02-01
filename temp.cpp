#include <lgpio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int keyGPIO_row[4]={21,22,23,24};
int keyGPIO_col[3]={25,26,27};
int keyMap[4][3]={{'1','2','3'},{'4','5','6'},{'7','8','9'},{'*','0','#'}};

#define NUMTHRED 2
void *numpad_handler(void *param);
void *led_running(void *param);

void initGPIO();
int  getch();
int  h;
int  levelHigh[4]={1,1,1,1};

void gpio_stop(int sig){
    printf("User pressing CTRL-C");
    lgGpiochipClose(h);
    exit(0);
}

int main(){
    int ch;

    initGPIO();
    signal(SIGINT,gpio_stop);
    printf("Please press a key on keypad:");
    fflush(stdout);

    int i;
	pthread_t tid[NUMTHRED];
    pthread_attr_t attr[NUMTHRED];
    void *(*thread[NUMTHRED])(void *)={numpad_handler, led_running};
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
    printf("Program terminated.\n");
    return 0;
}

void *numpad_handler(void *param){
    while(1){
        int ch = getch();
        if(ch != -1){
            printf("You pressed key: %c \n",ch);
        }
        usleep(100000);
    }
	pthread_exit(NULL);
}

void *led_running(void *param){
	while(count<10){
		printf("Key pressed : %2d\r",count);
		fflush(stdout);
		usleep(250000);
	}
	pthread_exit(NULL);
}


void initGPIO(){
    int i;
    if((h=lgGpiochipOpen(0)) < 0) exit(-1);

    //set GPIO group as output and high
    lgGroupClaimOutput(h,0,4,keyGPIO_row,levelHigh);

    //set GPIO group as input and enable pull-up resistor
    lgGroupClaimInput(h,LG_SET_PULL_UP,3,keyGPIO_col);
}

int getch(){
    int row,col;
    for(row=0;row<4;row++){
        lgGpioWrite(h,keyGPIO_row[row],0);
        for(col=0;col<3;col++){
            if(lgGpioRead(h,keyGPIO_col[col])==0) break;
        }
        if(col<3)break; // key pressing detected
        lgGpioWrite(h,keyGPIO_row[row],1);
    }
    if(row<4){
        while(lgGpioRead(h,keyGPIO_col[col])==0) usleep(100000);
        lgGpioWrite(h,keyGPIO_row[row],1);
        return keyMap[row][col];
    }
    return -1;
 
}
