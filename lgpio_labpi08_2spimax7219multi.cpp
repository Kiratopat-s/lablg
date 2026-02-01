#include <lgpio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

#define NOOP        0
#define DECODE_MODE 0x09
#define INTENSITY   0x0a
#define SCAN_LIMIT  0x0b
#define SHUTDOWN    0x0c
#define DISPLAY_TEST 0x0f
int h;

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

int main(){
	int i,j,k;
	int spi;

    if((h=lgGpiochipOpen(0)) < 0) exit(-1);
    if((spi=lgSpiOpen(0,0,100000,0))<0) return -1;
    for(k=0;k<4;k++){
        mySpiWriteX(spi,INTENSITY,10,k);
        mySpiWriteX(spi,DECODE_MODE,0,k);
        mySpiWriteX(spi,SCAN_LIMIT,7,k);
    }
    for(j=0;j<64;j++){
        for(k=0;k<4;k++)
            mySpiWriteX(spi,SHUTDOWN,0,k);
        for(i=0;i<8;i++)
            for(k=0;k<4;k++)
                mySpiWriteX(spi,i+1,1<<((i+j)%8),k);
        for(k=0;k<4;k++)
            mySpiWriteX(spi,SHUTDOWN,1,k);
        usleep(100000);
    }
    lgSpiClose(spi);
    lgGpiochipClose(h);
	return 0;
}
