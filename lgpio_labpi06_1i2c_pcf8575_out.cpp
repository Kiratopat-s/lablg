#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <lgpio.h>

int running=true;
int h;
void gpio_stop(int sig);

int main(){
    int fd,i;
    char data[2];

    if((h=lgGpiochipOpen(0)) < 0) exit(-1);
    if((fd= lgI2cOpen(1,0x20,0)) < 0) exit(-1);
    signal(SIGINT,gpio_stop);

    printf("I2C  PCF8575 16bit GPIO testing...\n");

    while(running){
        for(i=0;i<16;i++){
            //We use 1's complement as LED light up with 0
            //and turn off with 1 
            data[0]=~i;     //Write to port 0
            data[1]=0xff;   //Write to port 1
            lgI2cWriteDevice(fd,data,2);
            usleep(100000);

        }
        printf("."); fflush(stdout);
    }
    lgI2cClose(fd);
    lgGpiochipClose(h);
    return 0;
}

void gpio_stop(int sig){
    printf("Exiting..., please wait\n");
    running = false;
}
