#include <lgpio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

int keyGPIO_row[4]={21,22,23,24};
int keyGPIO_col[3]={25,26,27};
int keyMap[4][3]={{'1','2','3'},{'4','5','6'},{'7','8','9'},{'*','0','#'}};

int ledGPIO[4]={4,5,6,12};
int ledLevel[4]={0};

#define NUMTHRED 2
void *numpad_handler(void *param);
void *led_running(void *param);
void blinkingLED();

void initGPIOKeypad();
void initGPIOLED();
int  getch();
int  h;
int  levelHigh[4]={1,1,1,1};

int ch;
volatile int keynum = 1;
volatile bool running = true; 

void gpio_stop(int sig){
    printf("\nUser pressing CTRL-C\n");
    running = false;
}

int main(){
    int i,j;
    pthread_t tid[NUMTHRED];
    pthread_attr_t attr[NUMTHRED];
    void *(*thread[NUMTHRED])(void *)={numpad_handler, led_running};
    
    // Open GPIO chip once in main
    if((h=lgGpiochipOpen(0)) < 0) {
        printf("Failed to open GPIO chip\n");
        exit(-1);
    }
    
    initGPIOKeypad();
    initGPIOLED();
    
    signal(SIGINT, gpio_stop);

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

    for(j=0;j<4;j++)
        lgGpioWrite(h,ledGPIO[j],0);
   
    lgGpiochipClose(h);
    printf("Program terminated.\n");
    return 0;
}

void *numpad_handler(void *param){
    printf("Please press a key on keypad:");
    fflush(stdout);
    while(running){
        ch = getch();
        if(ch<0){
            usleep(100000);
            continue;
        }
        keynum = ch - '0';
        printf("Key number = %d\n", keynum);
        printf("\nKey = %c\n",ch);
        printf("ASCII = %d\n",ch);
        printf("Hex   = %X\n",ch);
        printf("ASCII INT = %d\n", ch-'0');
        printf("Please press a key on keypad:");
        fflush(stdout);
        if(ch=='#'){
            running = false;
            break;
        }
    }
    pthread_exit(NULL);
}

void *led_running(void *param){
    blinkingLED();
    pthread_exit(NULL);
}


void initGPIOKeypad(){
    //set GPIO group as output and high
    lgGroupClaimOutput(h,0,4,keyGPIO_row,levelHigh);

    //set GPIO group as input and enable pull-up resistor
    lgGroupClaimInput(h,LG_SET_PULL_UP,3,keyGPIO_col);
}

void initGPIOLED(){
   lgGroupClaimOutput(h,0,4,ledGPIO,ledLevel);
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

void blinkingLED(){
    int i,j;
    int pattern[2][4]={{1,0,1,0},{0,1,0,1}};

    while(running){
        for(i=0;i<2 && running;i++){
            for(j=0;j<4;j++)
                lgGpioWrite(h,ledGPIO[j],pattern[i][j]);
            usleep(100000 * keynum);
        }
    }
    for(j=0;j<4;j++)
        lgGpioWrite(h,ledGPIO[j],0);
}