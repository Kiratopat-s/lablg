#include <lgpio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#include "fontimage32x8.c"
#include <time.h>

#define NOOP        0
#define DECODE_MODE 0x09
#define INTENSITY   0x0a
#define SCAN_LIMIT  0x0b
#define SHUTDOWN    0x0c
#define DISPLAY_TEST 0x0f
int running = true;
int h;

void gpio_stop(int sig);
void mySpiWrite(int port,char mode,char value);
void mySpiWriteX(int port,char mode,char value,int location);
void mySpiWrite4(int port,char mode,char value);

int main(){
	int i;
	int spi;
	time_t rawtime;
	struct tm *timeinfo;
	char text[12];
	char data[8];

    if((h=lgGpiochipOpen(0)) < 0) exit(-1);
    if((spi=lgSpiOpen(0,0,100000,0))<0) return -1;
    signal(SIGINT,gpio_stop);

    mySpiWrite4(spi,INTENSITY,10);
    mySpiWrite4(spi,DECODE_MODE,0);
    mySpiWrite4(spi,SCAN_LIMIT,7);
    mySpiWrite4(spi,SHUTDOWN,0);

	while(running){
        time (&rawtime);
        timeinfo = localtime(&rawtime);
        sprintf(text,"%.2d%.2d%.2d",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);

		f_clear();
		for(i=0;i<6;i++){
            f_char(text[i]);
            if(i==1||i==3){
               f_loc-=2;
               f_char('\'');
               f_loc-=2;
            }
        }
//        mySpiWrite4(spi,SHUTDOWN,0);

		for(i=0;i<8;i++){
            data[0]=data[2]=data[4]=data[6]=i+1;
            data[7]=fimage[i]&0xff;
            data[5]=(fimage[i]>>8)&0xff;
            data[3]=(fimage[i]>>16)&0xff;
            data[1]=(fimage[i]>>24)&0xff;
            lgSpiXfer(spi,data,data,8);

//		    printf("fimage[%d]=%.8X\n",i,fimage[i]);
//			mySpiWriteX(spi,i+1,fimage[i]&0xff,0);
//			mySpiWriteX(spi,i+1,(fimage[i]>>8)&0xff,1);
//			mySpiWriteX(spi,i+1,(fimage[i]>>16)&0xff,2);
//			mySpiWriteX(spi,i+1,(fimage[i]>>24)&0xff,3);
		}
		mySpiWrite4(spi,SHUTDOWN,1);
		usleep(250000);

	}
    lgSpiClose(spi);
    lgGpiochipClose(h);
	return 0;
}

void mySpiWrite(int port,char mode,char value){
   char data[2];
   data[0]=mode;
   data[1]=value;
   lgSpiXfer(port,data,data,2);
}

void mySpiWriteX(int port,char mode,char value,int location){
   char data[8]={0};
   data[0]=mode;
   data[1]=value;
   lgSpiXfer(port,data,data,2+location*2);
}

void mySpiWrite4(int port,char mode,char value){
   char data[8];
   data[0]=data[2]=data[4]=data[6]=mode;
   data[1]=data[3]=data[5]=data[7]=value;
   lgSpiXfer(port,data,data,8);
}

void gpio_stop(int sig){
    printf("Exiting..., please wait\n");
    running = false;
}
