#include <lgpio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <signal.h>

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
void putImage(int port,char* data,int dlen,int offset);

char data[48]= {0xcf,0x49,0x4a,0x49,0x6b,0x00,0x0f,0x00,
                0xf6,0x97,0xd2,0x12,0xf2,0x00,0xe0,0x00,
                0x7a,0x4a,0x0a,0x4a,0xbb,0x00,0xf3,0x22,
                0x3d,0x25,0x2d,0x21,0xbb,0x00,0x00,0x01,
                0x9b,0x1d,0xc9,0x49,0x89,0x00,0x00,0x00,
                0xf1,0x91,0x91,0x90,0x9b,0x00,0x1f,0x01};
int main(){
	int i,k;
	int spi;

    if((h=lgGpiochipOpen(0)) < 0) exit(-1);
    if((spi=lgSpiOpen(0,0,100000,0))<0) return -1;
    signal(SIGINT,gpio_stop);

    mySpiWrite4(spi,INTENSITY,10);
    mySpiWrite4(spi,DECODE_MODE,0);
    mySpiWrite4(spi,SCAN_LIMIT,7);
    mySpiWrite4(spi,SHUTDOWN,0);

    for(k=0;k<8;k++){
        mySpiWriteX(spi,k+1,data[k],0);
        mySpiWriteX(spi,k+1,data[k+8],1);
        mySpiWriteX(spi,k+1,data[k+16],2);
        mySpiWriteX(spi,k+1,data[k+24],3);
    }
    mySpiWrite4(spi,SHUTDOWN,1);
    sleep(1);

	while(running){
		for(i=-31;i<48;i++){
		    if(!running)break;
            mySpiWrite4(spi,SHUTDOWN,0);

			putImage(spi,data,48,i);
			 mySpiWrite4(spi,SHUTDOWN,1);
			usleep(100000);
		}
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

void putImage(int port,char* data,int dlen,int offset){
// LED size = 32
	uint32_t fimage[8];

	uint16_t value=0;
	int j,k,joffset;

	for(j=0;j<4;j++){
        for(k=0;k<8;k++){
            if((j+(offset/8)<0)||(j+(offset/8)>=(dlen/8)))
                mySpiWriteX(port,k+1,0,j);
            else{
                value=0;
                joffset=offset+(j*8);
                if(joffset%8){ //loading previous dataset
                    if(joffset>=-7)
                       value|=data[(joffset/8)*8+(k%8)];
                    if(((joffset/8+1)<6)&&(joffset>=0))
                        value|=(data[(joffset/8+1)*8+(k%8)]<<8);
//                    printf("joffset=%d dindex=%d data=%.4X   ",joffset,(joffset/8)*8+(k%8),value);
//                       fflush(stdout);
                    if(joffset<0)
                    (value <<=(8-joffset)),value>>=8;
                    else
                    value >>=joffset%8;
                    printf("dataS=%.4X \n",value);

                    mySpiWriteX(port,k+1,value,j);
                }else{
                    value=data[(joffset/8)*8+(k%8)];
                    mySpiWriteX(port,k+1,value,j);
                }
            }
        }
	}
}

void gpio_stop(int sig){
    printf("Exiting..., please wait\n");
    running = false;
}
