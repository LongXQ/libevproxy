#include "event_file.h"
#include <stdlib.h>
#include <unistd.h>

int initFileEventProxy(event_file_proxy *proxy,int setsize){
	proxy->type=
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
	proxy->next_id=oldsetsize;
	_initEvents((event_base *)(events+oldsetsize),oldsetsize,oldsetsize);
	return EVENT_OK;
}

event_base *createFileEvent(event_proxy_base *ev_proxy,int fd,int mask,fileReadProc ev_read,fileWriteProc ev_write,void *clientData){
	event_file *event;
	if(ev_proxy && ev_proxy->type==EVENT_FILE){
		event_file_proxy *proxy=(event_file_proxy*)ev_proxy;
		if(proxy->setsize==proxy->elements){
			if(_resize(proxy)==EVENT_ERR){
				return NULL
			}
			if(event->ops=(event_file_operations*)malloc(sizeof(event_file_operations))==NULL){
				return NULL;
			}
			int next_id=proxy->events[proxy->next_id].id;
			event=&(proxy->events[proxy->next_id]);
			event->id=proxy->next_id;
			event->fd=fd;
			event->mask=mask;
			event->type=EVENT_FILE;
			event->flag=EV_NONE;
			event->ops->read=ev_read;
			event->ops->write=ev_write;
			event->ops->trigger=NULL;
			event->clientData=clientData;
			proxy->next_id=next_id;
			proxy->elements=proxy->elements+1;
		}
	}else{
		return NULL;
	}
}

int _removeEvent(event_file_proxy *proxy,event_file *ev){
	int id=ev_file->id;
	int nex_id=proxy->events[proxy->nex_id].id;
	proxy->events[proxy->nex_id].id=id;
	if(next_id==-1){
		ev->id=-1;
	}else{
		ev->id=nex_id;
	}
	free(ev->ops);
	ev->ops=NULL;
	free(ev->clientData);
	ev->clientData=NULL;
	ev->flag=EV_NONE;
	ev->fd=-1;
	proxy->elements=proxy->elements-1;
	_shrink(proxy);
	return EVENT_OK;
}

/* 收缩内存大小，防止浪费太多内存，但是由于每次变更内存，会导致事件存储的位置发生变化，
 *  所以是一个代价很高的操作，只有当浪费内存足够大的时候，才进行此操作
 */
void _shrink(event_file_proxy *proxy){
	double setsize=proxy->setsize;
	double elements=proxy->elements;
	if(elements/setsize>SHRINK_RATIO)
		return;
	...
	...
	...
}

