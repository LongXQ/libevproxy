#include "event_epoll.h"
#include "event.h"
#include <sys/epoll.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>

int createEventPoll(event_proxy *proxy,int setsize){
	event_poll *ev_poll=(event_poll*)malloc(sizeof(event_poll));
	if(!ev_poll)return EVENT_ERR;
	ev_poll->events=(struct epoll_event*)malloc(sizeof(struct epoll_event)*setsize);
	if(!ev_poll->events){
		free(ev_poll);
		return EVENT_ERR;
	}
	ev_poll->epfd=epoll_create(1024);
	if(ev_poll->epfd==-1){
		free(ev_poll->events);
		free(ev_poll);
		return EVENT_ERR;
	}
	proxy->ev_poll=ev_poll;
	return EVENT_OK;
}

int resizeEventPoll(event_proxy *proxy,int setsize){
	if(setsize<=proxy->setsize)return EVENT_ERR;
	event_poll *ev_poll=proxy->ev_poll;
	struct epoll_event *events;
	events=(struct epoll_event*)malloc(setsize*sizeof(struct epoll_event));
	//events=(struct epoll_event*)realloc(ev_poll->events,sizeof(struct epoll_event)*setsize);
	if(events==NULL)
		return EVENT_ERR;
	memcpy((void*)events,(void*)ev_poll->events,sizeof(struct epoll_event)*proxy->setsize);
	free(ev_poll->events);
	ev_poll->events=events;
	return EVENT_OK;
}

void deleteEventPoll(event_proxy *proxy){
	event_poll *ev_poll=proxy->ev_poll;
	close(ev_poll->epfd);
	free(ev_poll->events);
	free(ev_poll);
}

int addPollEvent(event_proxy *proxy,int fd,int mask){
	event_poll *ev_poll=proxy->ev_poll;
    struct epoll_event ee;

    int op = (proxy->events[fd])->mask==EVENT_NONE?
            EPOLL_CTL_ADD : EPOLL_CTL_MOD;
  
	ee.events = 0;
	mask |= (proxy->events[fd])->mask;
    if (mask & EVENT_READABLE) ee.events |= EPOLLIN;
    if (mask & EVENT_WRITEABLE) ee.events |= EPOLLOUT;
    ee.data.u64 = 0; 
    ee.data.fd = fd;
    if (epoll_ctl(ev_poll->epfd,op,fd,&ee) == -1) return EVENT_ERR;
    return EVENT_OK;
}

void delPollEvent(event_proxy *proxy,int fd,int delmask){
	event_poll *ev_poll=proxy->ev_poll;
	struct epoll_event ee;
	int mask=(proxy->events[fd])->mask & (~delmask);
	ee.events=0;
	if(mask & EVENT_READABLE) ee.events|=EPOLLIN;
	if(mask & EVENT_WRITEABLE) ee.events |= EPOLLOUT;
	ee.data.u64=0;
	ee.data.fd=fd;
	if(mask!=EVENT_NONE){
		epoll_ctl(ev_poll->epfd,EPOLL_CTL_MOD,fd,&ee);
	}else{
		epoll_ctl(ev_poll->epfd,EPOLL_CTL_DEL,fd,&ee);
	}
}

int eventPoll(event_proxy *proxy,struct timeval *tvp){
	event_poll *ev_poll=proxy->ev_poll;
	int retval,numevents=0;
	retval=epoll_wait(ev_poll->epfd,ev_poll->events,proxy->setsize,tvp?(tvp->tv_sec*1000+tvp->tv_usec/1000):-1);
	if(retval>0){
		int j=0;
		numevents=retval;
		for(j=0;j<numevents;j++){
			int mask=0;
			struct  epoll_event *e=ev_poll->events+j;
			if(e->events & EPOLLIN)mask|=EVENT_READABLE;
			if(e->events & EPOLLOUT)mask|=EVENT_WRITEABLE;
			if(e->events & EPOLLERR)mask|=EVENT_WRITEABLE;
			if(e->events & EPOLLHUP)mask|=EVENT_WRITEABLE;
			proxy->ev_fires[j].fd=e->data.fd;
			proxy->ev_fires[j].mask=mask;
		}
	}
	return numevents;
}

char *eventPollName(void){
	return "epoll";
}
