#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int c[4][4];
int a[4][4]={{1,2,3,4},{5,6,7,8},{9,10,11,12},{13,14,15,16}};
int b[4][4]={{10,-10,10,-10},{-10,10,-10,10},{10,-10,10,-10},{-10,10,-10,10}};

void showresult();
void *threadFunction(void *selector);

int main(void){
	pthread_t tid1,tid2;		// Thread ID
	pthread_attr_t attr1,attr2;  	// Thread attributes
        int section1=0,section2=1;

	pthread_attr_init(&attr1);  // Get default attributes
	pthread_attr_init(&attr2);  // Get default attributes

	// Create 2 threads
	pthread_create(&tid1,&attr1,threadFunction,(void *)&section1);
	pthread_create(&tid2,&attr2,threadFunction,(void *)&section2);

       // Wait until all threads finish
	pthread_join(tid1,NULL);
	pthread_join(tid2,NULL);

	pthread_attr_destroy(&attr1);
	pthread_attr_destroy(&attr2);

	showresult();
	return 0;
}

void showresult(){
	int i,j;

	for(j=0;j<4;j++){
		printf("|");
		for(i=0;i<4;i++){
			printf("%3d ",c[j][i]);
		}
		printf("|\n");
	}
}
void *threadFunction(void *selector){
	int sel=(int)*((int *)selector);
	int start,stop;
	int i,j;

	start=sel*2;
	stop =start+2;

	for(j=start;j<stop;j++)
		for(i=0;i<4;i++){
			c[j][i]=a[j][i]+b[j][i];
			// To Show how thread runs, this will slow thing down...
			sleep(1);
			printf("From thread%d i=%d j=%d\n",sel,i,j);
			fflush(stdout); // Send text to display immediately
		}
	pthread_exit(0);
}
