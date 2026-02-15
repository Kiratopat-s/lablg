
#include "pi.h"
#include <lgpio.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define setInput() lgGpioFree(h,DHT11_PIN);lgGpioClaimInput(h,LG_SET_PULL_UP,DHT11_PIN)
#define setOutputLow() lgGpioFree(h,DHT11_PIN);lgGpioClaimOutput(h,LG_SET_OUTPUT,DHT11_PIN,0)

void gpio_stop(int sig);
int running = 1;
int h;
#define  DHT11_PIN   17  // GPIO_17
#define  DHT11_DELAY 77  // Delay time for detecting 0 or 1  79
struct DHT11_data{
    float temp;
    float humidity;
};

int DHT11_Init();
int DHT11_read(struct DHT11_data *data);
#define DHT11_readOneByte(x)  {        \
   int _i,_j;                           \
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

int main(){
    char run[4]={'|','/','-','\\'};
    int step=0;
    struct DHT11_data data;
    DHT11_Init();
    signal(SIGINT,gpio_stop);
    while(running){
        if(DHT11_read(&data)){
            printf("Temp = %5.1fc, Humidity = % 5.1f%%  %c\r",
                    data.temp,data.humidity,run[step]);
            step = (step+1)&3;
            fflush(stdout);
        }else{
            usleep(5000);
            continue;
        }
         usleep(100000);
    }
	lgGpiochipClose(h);
    return 0;
}

int DHT11_Init(){
    if((h=lgGpiochipOpen(0)) < 0) exit(-1);
    setInput();
    return 0;
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

void gpio_stop(int sig){
    printf("User pressing CTRL-C");
    running = 0;
}

