/////////////////////////////////////////////////////////
//  pi.h
//  This header file contains functions to perform
//  precision timing and process control for Raspberry Pi3/4/5
/////////////////////////////////////////////////////////
#include <string.h>
#include <sched.h>
#include <time.h>

int piHiPri(const int pri);
void udelay(const long us);
void yield();


////////////////////////////////////////////////////////

int piHiPri(const int pri){
// Source code of this function by Gordon Henderson
    struct sched_param sched;
    memset(&sched,0,sizeof(sched));
    if(pri>sched_get_priority_max (SCHED_RR))
        sched.sched_priority = sched_get_priority_max (SCHED_RR);
        else
        sched.sched_priority = pri;
    return sched_setscheduler(0,SCHED_RR,&sched);
}

void udelay(const long us){
// Delay without yielding process
    long st;
    long tdif;
    struct timespec tnow;

    clock_gettime(CLOCK_REALTIME,&tnow);
    st = tnow.tv_nsec;
    while(1){
        clock_gettime(CLOCK_REALTIME,&tnow);
        tdif = tnow.tv_nsec - st;
        if(tdif < 0) tdif += 1000000000;
        if(tdif > (us*1000)) break;
    }
}

void yield(){
    sched_yield();
}