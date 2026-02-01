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
#include "pi.h"

#define setInput() lgGpioFree(h,DS18B20_PIN);lgGpioClaimInput(h,LG_SET_PULL_UP,DS18B20_PIN)
#define setOutputLow() lgGpioFree(h,DS18B20_PIN);lgGpioClaimOutput(h,LG_SET_OUTPUT,DS18B20_PIN,0)

#define TIMEWRITE   59  //default 60

void gpio_stop(int sig);
int running = 1;
int h;
#define  DS18B20_PIN   16  // GPIO_16
//#define  udelay(us) usleep(us)

//#define udelay(us) lguSleep(us/1000000.0)
int DS18B20_Init();
void DS18B20_Write(uint8_t data);
uint8_t DS18B20_Read(void);
uint8_t crc8(uint8_t *addr,uint8_t len);

int main(){
    uint8_t tempL,tempH;
    float temp;
    char run[4]={'|','/','-','\\'};
    int step=0;

    if((h=lgGpiochipOpen(0)) < 0) exit(-1);

    signal(SIGINT,gpio_stop);
    lgGpioClaimInput(h,LG_SET_PULL_UP,DS18B20_PIN);

    while(running){
        if(!DS18B20_Init()){
            printf("No DS18B20 connected!\n");
            sleep(1);
            continue;
        }
        udelay(1000);
        DS18B20_Write(0xCC);  //Skip rom
        DS18B20_Write(0x44);  //Read temperature

        udelay(100000);
        if(!DS18B20_Init ()){
            printf("No DS18B20 connected!           \r");
            sleep(1);
            continue;
        }
        udelay(1000);
        DS18B20_Write (0xCC);  // skip ROM
        DS18B20_Write (0xBE);  // read first two bytes from scratch pad
        
        uint8_t scratchpad[9];
        uint8_t CRC=0;
        piHiPri(99);
        for(uint8_t x=0;x<9;x++)
            scratchpad[x] = DS18B20_Read();
        piHiPri(0);
// Due to time inconsistency of Raspberry Pi, we opt to check
// For irregularity of temperature instead of checking for CRC
//        CRC = crc8(scratchpad,8);
//            printf ("CRC=%.2X scrCRC=%.2X   ",CRC,scratchpad[8]);
        tempL = scratchpad[0];
        tempH = scratchpad[1];
        temp = ((float)((tempH<<8)|tempL))/16;
        if(temp>125){
            //error reading
            udelay(50000);
            continue;
        }  
        printf("TH=%.2X TL=%.2X  Temperature=%6.2f %c\r",tempH,tempL,temp,run[step]);
        step=(step+1)&3;
       
        fflush(stdout);
        udelay(200000);
    }
	lgGpiochipClose(h);
    return 0;
}

int DS18B20_Init(){
    uint8_t response=0;

    setOutputLow();
    lgGpioWrite(h,DS18B20_PIN,0);
    udelay(480);    //Delay according to data sheet

    setInput();
    //Wait for DS18B20 to acknowledge;
    for(int i=100;i>0;i--){
        if(!lgGpioRead(h,DS18B20_PIN)){
            udelay(480);  //Wait until DS18B20 ready to receive command (480us)
            return 1;
        }
        udelay(2);
    }
    return 0;
}

void DS18B20_Write(uint8_t data){
    piHiPri(99);
	for (int i=0; i<8; i++)
	{
        setOutputLow();
        udelay (1);  // wait for 1 us
		if(data&1){
            lgGpioWrite(h,DS18B20_PIN, 1);  // pull the pin HIGH
			setInput(); 	    //Release Low pull
		    udelay(TIMEWRITE);  // wait for another 60 us   (1 us low + 59 us high pull floating)
		}else{
			udelay(TIMEWRITE);	 // wait for 60-120 us according to datasheet (60 us low pull)
			lgGpioWrite(h,DS18B20_PIN, 1);  // pull the pin HIGH
			setInput(); 	    //Release Low pull
		}
		data >>=1;
		udelay(5); // Wait for next bit
	}
    setInput(); //Release 1-wire bus
    piHiPri(0);
}

uint8_t DS18B20_Read(void){
	uint8_t value=0;
	for(int i=0;i<8;i++){
        setOutputLow();
        udelay(1);
        lgGpioWrite(h,DS18B20_PIN, 1); // pull the pin HIGH
        setInput();  // master releases 1-wire bus

        value >>=1;
//        udelay (5);  // wait for < 15us from the start of pulling LOW
        udelay (8);  // wait for < 15us from the start of pulling LOW
        if(lgGpioRead(h,DS18B20_PIN))           // CHeck 1-wire status
	   		value |= 0x80;  // read = 1
	   udelay (TIMEWRITE);  // wait for 50 us (The whole bit is atleast 60 us not including 1 us between bits)
	}
	return value;
}

uint8_t crc8(uint8_t *addr,uint8_t len){
    uint8_t crc = 0;
    while(len--){
        uint8_t inbyte = *addr++;
        for (uint8_t i=8; i; i--){
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if(mix) crc ^= 0x8c;
            inbyte >>=1;
        }
    }
    return crc;
}

void gpio_stop(int sig){
    printf("User pressing CTRL-C");
    running = 0;
}