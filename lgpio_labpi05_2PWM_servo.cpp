#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <lgpio.h>

int PWM_pin=18;
int h;

void gpio_stop(int sig);

int main(){
    int i;

    printf("Servo PWM (1000-2000 us positive pulse)\n");

    if((h=lgGpiochipOpen(0)) < 0) exit(-1);
    signal(SIGINT,gpio_stop);

    while(1){
        for(i=1000;i<2000;i+=20){
            lgTxServo(h,PWM_pin,i,50,0,0);
            usleep(20000);
        }
        for(i=2000;i>1000;i-=20){
            lgTxServo(h,PWM_pin,i,50,0,0);
            usleep(20000);
        }
    }
    return 0;
}

void gpio_stop(int sig){
    printf("User pressing CTRL-C");
    lgGpiochipClose(h);
    exit(0);
}
