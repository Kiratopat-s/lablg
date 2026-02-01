#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <lgpio.h>

int running=true;
int h;
void gpio_stop(int sig);

int main(){
    int fd;
	uint16_t data;

    if((h=lgGpiochipOpen(0)) < 0) exit(-1);
    if((fd= lgI2cOpen(1,0x48,0)) < 0) exit(-1);

    signal(SIGINT,gpio_stop);

    printf("I2C  ADS1115 4ch ADC testing...\n");
    //Single-End input/read from port0 A/D on
    //    0x03c3 Joystick Y-axis
    //    0x03d3 Joystick X-axis
    //    0x03e3 External input
    //    0x03f3 LDR
    while(running){
    	lgI2cWriteWordData(fd,1,0x03c3);
	while(((data=lgI2cReadWordData(fd,1))&0x80)==0){
//        printf("CTRL = %.4X\n",data);
        usleep(10000);
	}
        data = lgI2cReadWordData(fd,0);
        //reverse LO/HI byte
        printf("Vout = %.4X\r",((data>>8)&0xff)|((data<<8)&0xff00));
        fflush(stdout);
        usleep(50000);
    }
    lgI2cClose(fd);
    lgGpiochipClose(h);
    return 0;
}

void gpio_stop(int sig){
    printf("Exiting..., please wait\n");
    running = false;
}
