#include "font5x8.cpp"
#include <stdint.h>

uint32_t fimage[8];
int f_loc;

void f_clear(){
	int i;

	for(i=0;i<8;i++)
		fimage[i]=0;
	f_loc=0;
}

void f_char(char ch){
	int i;
	if(f_loc>27) f_loc=0;

	for(i=0;i<8;i++){
		    fimage[i] |= (uint32_t)font(ch,7-i)<<(f_loc);
	}
	f_loc+=5;
}
