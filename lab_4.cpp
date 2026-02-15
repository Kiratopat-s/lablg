#include <lgpio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include "pi.h"

int keyGPIO_row[4]={21,22,23,24};
int keyGPIO_col[3]={25,26,27};
int keyMap[4][3]={{'1','2','3'},{'4','5','6'},{'7','8','9'},{'*','0','#'}};

int ledGPIO[4]={4,5,6,12};
int ledLevel[4]={0};

#define NUMTHRED 4
void *numpad_handler(void *param);
void *led_running(void *param);
void blinkingLED();

void initGPIOKeypad();
void initGPIOLED();
int  getch();
int  h;
int  levelHigh[4]={1,1,1,1};

// ---- ultrasonic setup
#define MAXTIMEOUT 100000
#define MAXDISTANCE 20000
int trig=19;
int echo=20;
void *ultrasonic_handler(void *param);
void initGPIOUltrasonic();
int ultraSonic();
int average(int data[],int count);
double toCm(int distance);
int distanceStepToKeyNum(double distanceCm);
// ---- end of ultrasonic setup


// ---- PWM setup
volatile int pwm=50;  // Make volatile for thread safety
int PWM_pin=18;
void *servoPWM(void *param);
// ---- end of PWM setup

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
    void *(*thread[NUMTHRED])(void *)={numpad_handler, led_running, ultrasonic_handler, servoPWM};
    
    // // Open GPIO chip once in main
    // if((h=lgGpiochipOpen(0)) < 0) {
    //     printf("Failed to open GPIO chip\n");
    //     exit(-1);
    // }
    
    // initGPIOKeypad();
    initGPIOLED();
    initGPIOUltrasonic();
    
    signal(SIGINT, gpio_stop);

    for(i=0;i<NUMTHRED;i++){
        pthread_attr_init(&attr[i]);
        pthread_create(&tid[i],&attr[i],thread[i],NULL);
    }
    for(i=0;i<NUMTHRED;i++){
        pthread_join(tid[i],NULL);
    }

    for(i=0;i<NUMTHRED;i++){
        pthread_attr_destroy(&attr[i]);
    }

    for(j=0;j<4;j++) {
        lgGpioWrite(h,ledGPIO[j],0);
    }
        
   
    lgGpiochipClose(h);
    printf("Program terminated.\n");
    return 0;
}

void *ultrasonic_handler(void *param){
    int distance[8]={0};
    int delay;
    int count=0;
    double distanceCm;
    int step;

    // initGPIOUltrasonic();
    printf("Ultrasonic thread started\n");
    while(running){
        delay = ultraSonic();
        if(delay<0){
            usleep(10000);
            continue;
        }
        if(delay>MAXDISTANCE) delay = MAXDISTANCE;
        distance[count]= delay;
        count = (count+1)%8;
        delay = average(distance,8);
        distanceCm = toCm(delay);
        step = distanceStepToKeyNum(distanceCm);
        
        // Update servo position based on distance
        // Map distance: close (step 1-3) -> left (0-30%), medium (4-6) -> center (40-60%), far (7-9) -> right (70-100%)
        if(step <= 3){
            pwm = 10 + (step-1) * 10;  // 10-30% (left side)
        } else if(step <= 6){
            pwm = 40 + (step-4) * 10;  // 40-60% (center)
        } else {
            pwm = 70 + (step-7) * 10;  // 70-90% (right side)
        }
        
        // Ensure PWM stays within safe bounds for SG90 servo (0-100%)
        if(pwm < 0) pwm = 0;
        if(pwm > 100) pwm = 100;
        
        printf("Distance = %5d  (%5.1fcm) | step : %d | servo PWM: %d%%     \r",
            delay, distanceCm, step, pwm);
        fflush(stdout);
        usleep(50000);  // Increased delay for smoother servo movement
    }
    pthread_exit(NULL);
}

void *numpad_handler(void *param){
    // printf("Please press a key on keypad:");
    // fflush(stdout);
    while(running){
        ch = getch();
        if(ch<0){
            usleep(100000);
            continue;
        }
        keynum = ch - '0';
        // printf("Key number = %d\n", keynum);
        // printf("\nKey = %c\n",ch);
        // printf("ASCII = %d\n",ch);
        // printf("Hex   = %X\n",ch);
        // printf("ASCII INT = %d\n", ch-'0');
        // printf("Please press a key on keypad:");
        // fflush(stdout);
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
   lgGpioClaimOutput(h,0,PWM_pin,0);
}


void initGPIOUltrasonic(){
    if((h=lgGpiochipOpen(0)) < 0) exit(-1);

    //Set trigger pin as output
    lgGpioClaimOutput(h,0,trig,0);

    //Set echo pin as input
    lgGpioClaimInput(h,LG_SET_PULL_NONE,echo);
    //sleep(2);
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

int distanceStepToKeyNum(double distanceCm){
    int step = (int)(distanceCm / 10.0) + 1;
    return (step > 9) ? 9 : step;
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
    int stepDistance;
    int pattern[4][4]={{1,0,0,0},
                       {0,1,0,0},
                       {0,0,1,0},
                       {0,0,0,1}};

    while(running){
        for(i=0;i<4 && running;i++){
            stepDistance = distanceStepToKeyNum(toCm(ultraSonic()));
            for(j=0;j<4;j++){
                if (stepDistance <= 1) {
                    lgGpioWrite(h,ledGPIO[j],1);
                } else {
                    lgGpioWrite(h,ledGPIO[j],pattern[i][j]);
                }
            }
            // usleep(100000 * keynum);
            usleep(100000 * stepDistance);
        }
    }
    for(j=0;j<4;j++)
        lgGpioWrite(h,ledGPIO[j],0);
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