#ifndef EVENT_UTIL_HEAER
#define EVENT_UTIL_HEADER

#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>


void evGetTime(long  *seconds,long  *milliseconds);
long long evGetMillisecondsNow();
void evAddMillisecondsToNow(long long milliseconds,long *sec,long *ms);



#endif
