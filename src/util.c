#include "util.h"

void evGetTime(long  *seconds,long  *milliseconds){
		struct timeval tv;
		gettimeofday(&tv,NULL);
		*seconds=tv.tv_sec;
		*milliseconds=tv.tv_usec/1000;
}

long long evGetMillisecondsNow(){
		long sec,msec;
		evGetTime(&sec,&msec);
		long long ms=sec*1000+msec;
		return ms;
}

void evAddMillisecondsToNow(long long milliseconds,long *sec,long *ms){
		long cur_sec,cur_ms,when_sec,when_ms;
		evGetTime(&cur_sec,&cur_ms);
		when_sec=cur_sec+milliseconds/1000;
		when_ms=cur_ms+milliseconds%1000;
		if(when_ms>=1000){
				when_sec++;
				when_ms-=1000;
		}
		*sec=when_sec;
		*ms=when_ms;
}

