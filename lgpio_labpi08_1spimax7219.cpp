#include <lgpio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

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

int main(){
	int i,j;
	int spi;

    if((h=lgGpiochipOpen(0)) < 0) exit(-1);
    if((spi=lgSpiOpen(0,0,100000,0))<0) return -1;
    mySpiWrite(spi,INTENSITY,10);
    mySpiWrite(spi,DECODE_MODE,0);
    mySpiWrite(spi,SCAN_LIMIT,7);
    for(j=0;j<64;j++){
        mySpiWrite(spi,SHUTDOWN,0);
        for(i=0;i<8;i++)
            mySpiWrite(spi,i+1,1<<((i+j)%8));
        mySpiWrite(spi,SHUTDOWN,1);
        usleep(100000);
    }
    lgSpiClose(spi);
    lgGpiochipClose(h);
	return 0;
}
