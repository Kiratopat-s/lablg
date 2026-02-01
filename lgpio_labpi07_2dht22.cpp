#include <lgpio.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>
#include <string.h>
#include "pi.h"

#define setInput() lgGpioFree(h,DHT22_PIN);lgGpioClaimInput(h,LG_SET_PULL_UP,DHT22_PIN)
#define setOutputLow() lgGpioFree(h,DHT22_PIN);lgGpioClaimOutput(h,LG_SET_OUTPUT,DHT22_PIN,0)

void gpio_stop(int sig);
int running = 1;
int h;
#define  DHT22_PIN   17  // GPIO_17
#define  DHT22_DELAY 30  // Delay time for detecting 0 or 1
struct DHT22_data{
    float temp;
    float humidity;
};

int DHT22_Init();
int DHT22_read(struct DHT22_data *data);
#define DHT22_readOneByte(x)  {        \
   int _i,_j;                  \
   for(_i=0;_i<8;_i++){                 \
        for(_j=0;_j<100;_j++){          \
            if(lgGpioRead(h,DHT22_PIN)==1)break; \
            udelay(1);                  \
        }                               \
        udelay(DHT22_DELAY);            \
        x <<=1;                         \
        if(lgGpioRead(h,DHT22_PIN))         \
            x|=1;                       \
        for(_j=0;_j<100;_j++){          \
            if(lgGpioRead(h,DHT22_PIN)==0)break; \
            udelay(1);                  \
        }                               \
   }}

int main(){
    struct DHT22_data data;
    char run[4]={'|','/','-','\\'};
    int step=0;

    DHT22_Init();
    signal(SIGINT,gpio_stop);
    while(running){
         if(DHT22_read(&data)){
            printf("Temp = %5.1fc, Humidity = % 5.1f%%  %c\r",
                   data.temp,data.humidity,run[step]);
            step = (step+1)&3;
            fflush(stdout);
            usleep(200000);
         }else usleep(100000);
    }
	lgGpiochipClose(h);
    return 0;
}

int DHT22_Init(){
    if((h=lgGpiochipOpen(0)) < 0) exit(-1);
    setInput();
    return 0;
}

int DHT22_read(struct DHT22_data *data){
    int i;
    uint8_t temp_l,temp_h,hum_l,hum_h,crc;
    uint16_t temp,hum;
    //Sending Start signal
    hum_h=hum_l=temp_h=temp_l=crc=0;
    piHiPri(99);
	setOutputLow();
    udelay(1000);
    lgGpioWrite(h,DHT22_PIN,1);
    udelay(40);
    //waiting for response
    setInput();
    for(i=0;i<100;i++){       // pull low cycle
        if(lgGpioRead(h,DHT22_PIN)==1) break;
        udelay(1);
    }
    for(i=0;i<100;i++){       //pull high cycle
        if(lgGpioRead(h,DHT22_PIN)==0) break;
        udelay(1);
    }
    // Read data
    DHT22_readOneByte(hum_h);
    DHT22_readOneByte(hum_l);
    DHT22_readOneByte(temp_h);
    DHT22_readOneByte(temp_l);
    DHT22_readOneByte(crc);

    piHiPri(0);
//printf("hum_h = %.2X  hum_l = %.2X temp_h = %.2X temp_l = %.2X crc= %.2X\n",hum_h,hum_l,temp_h,temp_l,crc);
//fflush(stdout);
    // Check if data is valid
    if(((hum_h+hum_l+temp_h+temp_l)&0xff)!=crc)
        return 0;

    hum = (((uint16_t)hum_h)<<8)|(uint16_t)hum_l;
    temp = (((uint16_t)temp_h)<<8)|(uint16_t)temp_l;
    data->humidity = (float)hum/10.0;
    if(temp&0x8000) data->temp = -((float)(temp&0x7fff))/10.0;
    else data->temp = (float)temp/10.0;

    return 1;
}

void gpio_stop(int sig){
    printf("User pressing CTRL-C");
    running = 0;
}
