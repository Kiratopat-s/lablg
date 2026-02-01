#include <lgpio.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "pi.h"

#define DHT11_PIN	17
#define DHT11_DELAY  74 //79  76
#define setInput() lgGpioFree(h,DHT11_PIN);lgGpioClaimInput(h,LG_SET_PULL_UP,DHT11_PIN)
#define setOutputLow() lgGpioFree(h,DHT11_PIN);lgGpioClaimOutput(h,LG_SET_OUTPUT,DHT11_PIN,0)

struct DHT11_data{
	float temp;
	float humidity;
}data;

int gpioLED[4] ={4,5,6,12};
int running=true;
int h;

void initGPIO();
void gpio_stop(int sig);

int DHT11_read(struct DHT11_data *data);
#define DHT11_readOneByte(x)  {        \
   int _i,_j;                  \
   for(_i=0;_i<8;_i++){                 \
        for(_j=0;_j<100;_j++){          \
            if(lgGpioRead(h,DHT11_PIN)==0)break; \
            udelay(1);                  \
        }                               \
        udelay(DHT11_DELAY);           \
        x <<=1;                         \
        if(lgGpioRead(h,DHT11_PIN))     \
            x|=1;                       \
   }}

void *checkDistance(void *param);
void *showLED(void *param);

int main(){
    pthread_t tid[2];
    pthread_attr_t attr[2];
    void *(*thread[2])(void *)={checkDistance,showLED};
    int i;

    initGPIO();
    signal(SIGINT,gpio_stop);

    for(i=0;i<2;i++){
        pthread_attr_init(&attr[i]);
        pthread_create(&tid[i],&attr[i],thread[i],NULL);
    }

    printf("Waiting all threads to stop...\n");
    fflush(stdout);
    for(i=0;i<2;i++){
        pthread_join(tid[i],NULL);
    }
    for(i=0;i<2;i++){
        pthread_attr_destroy(&attr[i]);
    }
    lgGpiochipClose(h);
    return 0;
}

void initGPIO(){
    int zero[4]={0};

    if((h=lgGpiochipOpen(0)) < 0) exit(-1);
    setInput();
    lgGroupClaimOutput(h,LG_SET_OUTPUT,4,gpioLED,zero);
}

void *checkDistance(void *param){
    char run[4]={'|','/','-','\\'};
    int step=0;
	while(running){
		if(DHT11_read(&data)){
            printf("Temp = %5.1fc, Humidity = % 5.1f%%  %c\r",
                    data.temp,data.humidity,run[step]);
            step = (step+1)&3;
        }
        fflush(stdout);
        usleep(100000);
	}
    pthread_exit(NULL);
}

void *showLED(void *param){
    while(running){
        if(data.temp<21) lgGpioWrite(h,gpioLED[0],0);
        else lgGpioWrite(h,gpioLED[0],1);
        if(data.temp<25) lgGpioWrite(h,gpioLED[1],0);
        else lgGpioWrite(h,gpioLED[1],1);
        if(data.temp<27) lgGpioWrite(h,gpioLED[2],0);
        else lgGpioWrite(h,gpioLED[2],1);
        if(data.temp<29) lgGpioWrite(h,gpioLED[3],0);
        else lgGpioWrite(h,gpioLED[3],1);
        usleep(100000);
    }
	pthread_exit(NULL);
}

void gpio_stop(int sig){
    printf("Exiting..., please wait\n");
    running = false;
}

int DHT11_read(struct DHT11_data *data){
    int i;
    uint8_t temp_l,temp_h,hum_l,hum_h,crc;
    char tmp[16];
    //Sending Start signal
    hum_h=hum_l=temp_h=temp_l=crc=0;

    piHiPri(50);

    setOutputLow();
    udelay(18000);
    lgGpioWrite(h,DHT11_PIN,1);
    //waiting for response
    setInput();
    for(i=0;i<100;i++){
        if(lgGpioRead(h,DHT11_PIN)==0) break;
        udelay(1);
    }
    for(i=0;i<100;i++){
        if(lgGpioRead(h,DHT11_PIN)==1) break;
        udelay(1);
    }
    // Read data
    DHT11_readOneByte(hum_h);
    DHT11_readOneByte(hum_l);
    DHT11_readOneByte(temp_h);
    DHT11_readOneByte(temp_l);
    DHT11_readOneByte(crc);
    piHiPri(0);

//printf("hum_h = %.2X  hum_l = %.2X temp_h = %.2X temp_l = %.2X crc= %.2X \n",hum_h,hum_l,temp_h,temp_l,crc);
//fflush(stdout);

// Check if data is valid
    if(((hum_h+hum_l+temp_h+temp_l)&0xff)!=crc)
        return 0;

    sprintf(tmp,"%u.%u",hum_h,hum_l);
    data->humidity = atof(tmp);
    sprintf(tmp,"%u.%u",temp_h,temp_l);
    data->temp = atof(tmp);

    return 1;
}
