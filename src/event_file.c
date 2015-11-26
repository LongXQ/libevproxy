#include "event_epoll.h"
#include "event_file.h"
#include "event.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

void initEventFile(event_base *ev){
	event_file *evfile=(event_file*)ev;
	evfile->type=EVENT_FILE;
	evfile->flag=EVENT_ACTIVE;
	evfile->fd=-1;
	evfile->mask=EVENT_NONE;
	evfile->rfileProc=NULL;
	evfile->wfileProc=NULL;
	evfile->clientData=NULL;
}

event_base *createEventFile(event_proxy *proxy,int fd){
	if(fd>proxy->setsize){
			if(resizeProxySetsize(proxy,fd+DEFAULT_PROXY_INTERVAL)==EVENT_ERR)
			return NULL;
	}
	event_storage_manager *manager=proxy->ev_file_manager;
	event_base *ev=GetEventFromStorage(manager);
	if(ev==NULL)
		return NULL;
	initEventFile(ev);
	ev->fd=fd;
	proxy->events[fd]=ev;
	proxy->elements++;
	if(fd>proxy->maxfd)
		proxy->maxfd=fd;
	return ev;
}


int addFileEvent(event_proxy *proxy,event_base *ev,int mask,evFileProc *fileProc,void *clientData){
	if(ev==NULL){
		return EVENT_ERR;
	}
	if(addPollEvent(proxy,ev->fd,mask)==EVENT_ERR){
		return EVENT_ERR;
	}
	event_file *evfile=(event_file*)ev;
	evfile->mask |= mask;
    if (mask & EVENT_READABLE) evfile->rfileProc = fileProc;
    if (mask & EVENT_WRITEABLE) evfile->wfileProc = fileProc;
    evfile->clientData=clientData;
    return EVENT_OK;
}






