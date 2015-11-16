#include "event_interface.h"
#include "event_file.h"
#include "event_signal.h"
#include "event_timeout.h"
#include "event_user.h"

void _initEvents(event_base *events,int setsize,int prevsetsize){
	int interval=0;
	if(events->type==EVENT_FILE)
		interval=sizeof(event_file);
	if(events->type==EVENT_SIGNAL)
		interval=sizeof(event_sig);
	if(events->type==EVENT_TIMEOUT)
		interval=sizeof(event_timeout);
	if(events->type==EVENT_USER)
		interval=sizeof(event_user);

	event_base *prev=&events[0];
	for(int i=1;i<setsize;i++){
		prev->id=prevsetsize+i;
		prev=(event_base*)((void*)events+i*interval);
	}
	prev->id=-1;
}