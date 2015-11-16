#include "event_file.h"
#include <stdlib.h>
#include <unistd.h>

static void _initFileEvents(event_file *events,int setsize){
	for(int i=0;i<setsize;i++){
		events[i].id=-1;
		events[i].fl
	}
}
static int _resize(event_file_proxy *proxy){
	int oldsetsize=proxy->setsize;
	int newsetsize=2*oldsetsize;
	event_file *events;
	if((events=realloc(proxy->events,newsetsize*sizeof(event_file)))==NULL){
		return EVENT_ERR;
	}
	proxy->events=events;
	proxy->setsize=newsetsize;
	_initFileEvents(events+oldsetsize,oldsetsize);
	return EVENT_OK;
}
event_base *createFileEvent(event_proxy_base *ev_proxy,int fd,int mask,fileReadProc ev_read,fileWriteProc ev_write,void *clientData){
	event_file *events;;
	if(ev_proxy && ev_proxy->type==EVENT_FILE){
		event_file_proxy *proxy=(event_file_proxy*)ev_proxy;
		events=proxy->events;
		if(proxy->setsize==proxy->elements || (proxy->elements[proxy->next_id]).id==-1){
			_resize(proxy);
		}


	}else{
		return NULL;
	}
}