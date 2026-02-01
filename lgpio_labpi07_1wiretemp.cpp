#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <time.h>

// This example requires 1-wire to be enabled 
// DS18B20 must be connected to GPIO4 or
// you can set the pin using sudo dtoverlay w1-gpio gpiopin=<NUMBER> pullup=0

int main(){
    char path[64] = "/sys/bus/w1/devices/";
    char rom[20];
    char buf[256];
    DIR *dirp;
    struct dirent *direntp;
    int fd = -1;
    char *temp;
    float value;
    // mount 1wire if not exists
    system ("sudo modprobe w1-gpio");
    // mount DS-dbx20 temperature sensor
    system ("sudo modprobe w1-therm");

    //Search for temperature device
    if((dirp = opendir(path))==NULL){
       printf("Error connecting 1-wire temperature sensor!\n");
       return -1;
    }
    while((direntp = readdir(dirp))!=NULL)
    {
        if(strstr(direntp->d_name,"28-0")){
            strcpy(rom,direntp->d_name);
            printf("Found temperature device at %s\n",rom);
        }
    }
    closedir(dirp);
    //Identified target file to be read
    strcat (path,rom);
    strcat (path,"/w1_slave");

    while(1){
        if((fd=open(path,O_RDONLY))<0){
            printf("Error reading temperature device\n");
            return -1;
        }
        if(read(fd,buf,sizeof(buf))<0){
            printf("Read error\n");
            return -1;
        }
        // Search for location of t=xxxxx
        temp = strchr(buf,'t');
        sscanf(temp,"t=%s",temp);
        value =atof(temp)/1000;
        printf("temp : %3.3f c\n",value);
        sleep(1);
    }
    return 0;
}
