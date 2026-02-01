#include <stdio.h>
#include <stdlib.h>
#include <lgpio.h>

int h;  // gpiochip handle

char bitName[32][16]={"Reserved","Output","Active low","Open drain",
                      "Open source","Pull-up","Pull-down","Pull-none",
                      "LG:Input","LG:Output","LG:Alert","LG:Group",
                      "LG:Reserved","LG:Reserved","LG:Reserved","LG:Reserved",
                      "Input","Rising-Trigged","Falling-Trigged","RTC-Trigged"};

int main(){

    //Start using gpiochip0
    if((h=lgGpiochipOpen(0))<0) exit(-1);

    //Get the chip infomation
    lgChipInfo_t info;
    lgGpioGetChipInfo(h,&info);
    printf("Line total: %d, Name: %s, Label: %s\n",
           info.lines, info.name, info.label);

    //Get information for each line (GPIO pin)
    printf("==========================================\n");
    int i;
    lgLineInfo_t lInfo;
    for(i=0;i<info.lines;i++){
        lgGpioGetLineInfo(h,i,&lInfo);
        printf("Line:%3d | GPIO%.2d | Flags:%8X | name=%-16s | user=%s\n         ",
                i,lInfo.offset,lInfo.lFlags, lInfo.name,lInfo.user);
        for(int j=0;j<20;j++){
            if((lInfo.lFlags>>j)&1)
                printf("| %s ",bitName[j]);
        }
        printf("\n");
    }

    //Release gpiochip0
    lgGpiochipClose(h);
    return 0;
}