#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>


typedef struct event_base {
		int a;
		int b;
		int c;
		struct timeval tm;
		void *clientdata;
		void *readpro;
}event_base;

int main(){
		int i=10;
		int size=10;
		int oldsize=size;
		 event_base *base=(event_base*)malloc(sizeof(event_base)*size);
		while(i){
				oldsize=size;
				size*=10;
				base=(event_base*)realloc(base,size*sizeof(event_base));
				i--;
		}
		return 0;
}
