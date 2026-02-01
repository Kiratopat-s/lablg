#include <lgpio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int ledGPIO[4] = {4,5,6,12};
int ledLevels[4] = {0};
int h;
int running[4][4] = {{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}};

int main(){
    int i, j, k;
    if((h=lgGpiochipOpen(0)) < 0) exit(-1);

    //Output with initial high
//    for(i=0;i<4;i++)
//       lgGpioClaimOutput(h,0,ledGPIO[i],1);
    //We can set all of them at once
    lgGroupClaimOutput(h,0,4,ledGPIO,ledLevels);

    usleep(100000);
    for(k=0;k<1000;k++) {
        for(j=0;j<4;j++)
        {
            for(i=0;i<4;i++)
            {
                lgGpioWrite(h,ledGPIO[i], running[j][i]);
            }
            usleep(100000);
        }
            
    }
    
    // for(i=0;i<4;i++)
    //     lgGpioWrite(h,ledGPIO[i],1);

    // for(i=0;i<4;i++)
    //     lgGpioWrite(h,ledGPIO[i],0);

    lgGpiochipClose(h);
    return 0;
}
