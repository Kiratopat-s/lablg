#include <lgpio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int key = 25;
int h;

int main(){
    if((h=lgGpiochipOpen(0)) < 0) exit(-1);

    //Set as input with pull-up resistor enabled
    lgGpioClaimInput(h,LG_SET_PULL_UP,key);
    printf("Set G25 as input\n");
	printf("Value = %d\n",lgGpioRead(h,key));
    lgGpiochipClose(h);
    return 0;
}






