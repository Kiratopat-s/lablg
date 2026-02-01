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

    printf("LED PWM (0%%-100%% duty cycle)\n");

    if((h=lgGpiochipOpen(0)) < 0) exit(-1);
    signal(SIGINT,gpio_stop);

    while(1){
        for(i=0;i<100;i++){
            lgTxPwm(h,PWM_pin,50,i,0,0);    
            usleep(10000);
        }
        for(i=99;i>0;i--){
            lgTxPwm(h,PWM_pin,50,i,0,0);
            usleep(10000);
        }
    }
    return 0;
}

void gpio_stop(int sig){
    printf("User pressing CTRL-C");
    lgGpiochipClose(h);
    exit(0);
}
