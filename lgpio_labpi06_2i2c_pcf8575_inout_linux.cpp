#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>

int i2cInit(int busID);
void i2cError(int rw,int size);
void i2cWrite(int fd,const void *data,int size);
void i2cRead(int fd,void *data,int size);

int main(){
    int fd,i;
	uint8_t data[2];
	uint8_t flag;

	if((fd=i2cInit(0x20))==-1) exit(-1);

    printf("I2C  PCF8575 16bit GPIO In/Out (using linux functions) testing...\n");
    while(1){
		data[0]=data[1]=0xff;
		i2cWrite(fd,data,2);        //Set high state for acting as input
        i2cRead(fd,data,2);
        flag = data[1];

        printf("%.2X%.2X\n",data[0],data[1]);
        for(i=0;i<16;i++){
            i2cRead(fd,data,2);
            flag = data[1];

            data[0]=(~i) & flag;
            data[1]=0xff;       //Set P1 to 0xff for next read
			i2cWrite(fd,data,2);
            usleep(100000);
        }
        printf("."); fflush(stdout);
    }
    return 0;
}

int i2cInit(int busID){
    char devname[] = "/dev/i2c-1"; //Raspberry pi i2c port 1
    int fd;

    if((fd=open(devname,O_RDWR))<0){
        fprintf(stderr,"Fail to connect to the i2c device.\n");
        exit(-1);
    }
    if(ioctl(fd, I2C_SLAVE, busID)<0){
        fprintf(stderr,"Unable to communicate to the i2c device.\n");
        exit(-1);
    }
    return fd;
}

void i2cError(int rw,int size){
    if(rw){
        fprintf(stderr,"Error reading %d byte%s from i2c.\n",size,(size>1)?"s":"");
        exit(-1);
    }
    fprintf(stderr,"Error writing %d byte%s to i2c.\n",size,(size>1)?"s":"");
    exit(-1);
}

void i2cWrite(int fd,const void *data,int size){
    if((write(fd,data,size))!=size)
        i2cError(0,size);
}

void i2cRead(int fd,void *data,int size){
    if((read(fd,data,size))!=size)
        i2cError(1,size);
}
