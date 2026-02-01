#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>

int i2cInit(int busID);
void i2cError(int rw,int size);
void i2cWrite(int fd,const void *data,int size);
void i2cRead(int fd,void *data,int size);

int main(){
    int fd;
    unsigned char data[4];

    if((fd=i2cInit(0x48))==-1){
        return -1;
    }
    printf("I2C ADS1115 4-ch ADCtesting (Using Linux I/O)...\n");

    while(1){
			//Start single conversion
            //    0xc3 Joystick Y-axis
            //    0xd3 Joystick X-axis
            //    0xe3 External input
            //    0xf3 LDR
			data[0]=1;
			data[1]=0xc3;
			data[2]=0x03;
            i2cWrite(fd,data,3); //Write config register
			data[0] = 0;
			while((data[0] & 0x80) == 0){
				i2cRead(fd,data,2); //Check OS(ready) bit
                usleep(10000);
	        }
			//Read result
			data[0] = 0; // Read from conversion register
			i2cWrite(fd,data,1);
			i2cRead(fd,data,2); // Read two bytes
            printf("Vout = %.2X%.2X\r",data[0],data[1]);
            fflush(stdout);
            usleep(100000);
    }

    return 0;
}


int i2cInit(int busID){
    char devname[] = "/dev/i2c-1"; //Raspberry pi 2/3
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
