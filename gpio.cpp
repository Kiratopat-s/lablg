#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

char headerName[40][8] = {"3.3v","5v","I2C1SDA","5v","I2C1SCL","GND","4","TX","GND","RX",
						  "17","18","27","GND","22","23","3.3v","24","MOSI","GND",
						  "MISO","25","SCLK","CS0","GND","CS1","RESRV","RESRV","5","GND",
						  "6","12","13","GND","19","16","26","20","GND","21"};
int headerMap[40] =       {-1,-1,-2,-1,-2,-1,4,-2,-1,-2,
						  17,18,27,-1,22,23,-1,24,-2,-1,
						  -2,25,-2,-2,-1,-2,-2,-2,5,-1,
						  6,12,13,-1,19,16,26,20,-1,21};
/*
char headerName[40][8] = {"3.3v","5v","2","5v","3","GND","4","14","GND","15",
						  "17","18","27","GND","22","23","3.3v","24","10","GND",
						  "9","25","11","8","GND","7","0","1","5","GND",
						  "6","12","13","GND","19","16","26","20","GND","21"};
int headerMap[40] =       {-1,-1,2,-1,3,-1,4,14,-1,15,
						  17,18,27,-1,22,23,-1,24,10,-1,
						  9,25,11,8,-1,7,0,1,5,-1,
						  6,12,13,-1,19,16,26,20,-1,21};
*/
#define PINTOTAL 40
#define LINETOTAL 28

char piModel[32][6]={"A","B","A+","B+","2B","Alpha","CM1","-",
				     "3B","Zero","CM3","-","ZeroW","3B+","3A+","-",
					 "CM3+","4b","Z2W","400","CM4","CM4S","-","5",
					 "CM5","500","CM5L"};

typedef struct{
	int line;
	char used[32];
	char name[32];
	char mode[32];
	int isAlt;
	int isLevel;
}pinDataType;

pinDataType pinData[40];
char modeBlank[16]= "       ";

char *pinName(int pin);
char *pinMode(int pin);
int pinValue(int pin);
int gpioHardwareRevision();
void getAllPin();
void reminder();

int main(){
    int hwRevision,h,i;

    hwRevision = gpioHardwareRevision()>>4;
    hwRevision %=32;

	getAllPin();
	reminder();

    printf("+-----+------------+--------+---+ PI %-5s +---+--------+------------+-----+\n",piModel[hwRevision]);
    printf("| BCM |    Name    |  Mode  | V |  Board   | V |  Mode  | Name       | BCM |\n");
    printf("+-----+------------+--------+---+----++----+---+--------+------------+-----+\n");
    for(i=0;i<40;i+=2){
//     odd pin
        h = i;
        if((headerMap[h]<0)||(pinData[headerMap[h]].isAlt)){
            printf("|     | %10s |        |   |",pinName(h));
        }else if(pinData[headerMap[h]].isLevel){
			printf("| G%.2d | %10.10s | %6s | %.1d |",headerMap[h],pinName(h),pinMode(h),pinValue(h));
		}else{
            printf("| G%.2d | %10.10s | %6s | - |",headerMap[h],pinName(h),pinMode(h));
        }
        printf(" %2d || %2d ",i+1,i+2);
//     even pin
        h = i+1;
        if((headerMap[h]<0)||(pinData[headerMap[h]].isAlt)){
            printf("|   |        | %10s |     |\n",pinName(h));
        }else if(pinData[headerMap[h]].isLevel){
			printf("| %.1d | %6s | %10.10s | G%.2d |\n",pinValue(h),pinMode(h),pinName(h),headerMap[h]);
		}else{
            printf("| - | %6s | %10.10s | G%.2d |\n",pinMode(h),pinName(h),headerMap[h]);
        }
    }
    printf("+-----+------------+--------+---+----++----+---+--------+------------+-----+\n");
    printf("| BCM |    Name    |  Mode  | V |  Board   | V |  Mode  | Name       | BCM |\n");
    printf("+-----+------------+--------+---+ PI %-5s +---+--------+------------+-----+\n",piModel[hwRevision]);
    return 0;
}

char *pinName(int pin){
    if(headerMap[pin]<0) return headerName[pin];
	if(!pinData[headerMap[pin]].isLevel) 
		return pinData[headerMap[pin]].used;
	return pinData[headerMap[pin]].name;
}

char *pinMode(int pin){
    if(headerMap[pin]<0)
		return modeBlank;
    return pinData[headerMap[pin]].mode;
}

int pinValue(int pin){
	if(headerMap[pin]<0)
		return -1;
// Do not read value from non GPIO pin
	if(strstr(pinData[headerMap[pin]].name,"GPIO")==NULL)
		return -1;
// read value data
	FILE *fp;
	char command[256];
	sprintf(command,"gpioget /dev/gpiochip0 %d",headerMap[pin]);
	if((fp=popen(command,"r"))==NULL)
		return -1; // Error executing command
	if(fgets(command,256,fp)== NULL)
		return -1; // error reading data
	int i;
	for(i=0;(i<256)&&(command[i]>0x20);i++);
	command[i]=0;
	pclose(fp);
	return atoi(command);
}

int gpioHardwareRevision(){
	int raspberryPiRevision=-1;
	FILE *f = fopen("/proc/cpuinfo","r");
	if(f==NULL){
		perror("/proc/cpuinfo");
		return -1;
	}
	char entry[80];

	while(fgets(entry,sizeof(entry),f)!= NULL){
		if(strncmp("Model",entry,5)==0){
			printf("%s",entry);
		}
		if(strncmp("Revision",entry,8)==0){
			int i;
			for(i=0;(i<32)&&(entry[i]!=':');i++);
			if(i>=32) return -1;
			i++;
			printf("%s",entry);
			raspberryPiRevision = strtol(&entry[i],NULL,16);
		}
	}
	fclose(f);
	return raspberryPiRevision;
}

void getAllPin(){
	FILE *fp;
	char temp[256];

	//clear info
	for(int i=0;i<40;i++){
		pinData[i].line=-1;
		pinData[i].name[0]=0;
		pinData[i].used[0]=0;
		pinData[i].mode[0]=0;
		pinData[i].isLevel=1;
		pinData[i].isAlt=0;
	}

	if((fp=popen("gpioinfo /dev/gpiochip0","r"))==NULL){
		printf("Raspberry Pi not found\n");
		exit(-1);
	}
	// retrive first line
	fgets(temp,256,fp);
	// use data from second line onwards
	while(fgets(temp,256,fp)!=NULL){
		int flag=false;
		int i,j;
		for(i=0;(i<256)&&(temp[i]<0x20);i++);
		// found the word line
		i+=6;  //line number
		j=i+2; //colon
		temp[j]=0;
		int line = atoi(&temp[i]);
		if((line<0)||(line>27)) continue;  //read error
		//Get the first double quote
		for(i=j+1;(i<256)&&(temp[i]!='\"');i++);
		//Get the second double quote
		for(j=i+1;(j<256)&&(temp[j]!='\"');j++);
		temp[j]=0;
		pinData[line].line=line;
		strncpy(pinData[line].name,&temp[i]+1,31);
		//Search for used label
		for(i=j+1;(i<256)&&(temp[i]<=0x20);i++);
		//if the label is under double qoutes, find another
		if(temp[i]=='\"'){
			for(j=i+1;(j<256)&&(temp[j]!='\"');j++);
			pinData[line].isLevel=false;
			pinData[line].isAlt=true;
			i++;
		}
		else
			for(j=i+1;(j<256)&&(temp[j]>0x20);j++);
		temp[j]=0;
		strncpy(pinData[line].used,&temp[i],31);
		//Search for mode 
		for(i=j+1;(i<256)&&(temp[i]<=0x20);i++);
		for(j=i+1;(j<256)&&(temp[j]>0x20);j++);
		temp[j]=0;
		strncpy(pinData[line].mode,&temp[i],31);

		if(strstr(pinData[line].name,"GPIO")==NULL){
			pinData[line].isAlt = true;
		}
	}
	pclose(fp);
}

void reminder(){
	FILE *fp;
	char temp[256];
	if((fp=popen("gpioinfo -v","r"))!=NULL){
	printf("=========================================================\n");
		while(fgets(temp,255,fp)!=NULL){
			printf("%s",temp);
		}
		pclose(fp);
	}
	printf("=========================================================\n");
	printf("NOTICE: Please note that this program uses commands belonged to \n");
	printf("the older version of libgpiod (libgpiod V1), which does not support \n");
	printf("reading port value without changing port status or mode (all outputs will be)\n");
	printf("changed into inputs after running this program\n");
	printf("=========================================================\n");
}