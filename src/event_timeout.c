#include "event_timeout.h"
#include "event_epoll.h"
#include "util.h"
#include "event.h"
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

int initEventTimeout(event_base *ev,int fd[2],long sec,long msec){
	event_timeout *evtime=(event_timeout*)ev;
	evtime->type=EVENT_TIMEOUT;
	evtime->flag=EVENT_TRIGGERABLE | EVENT_ACTIVE;
	evtime->mask=EVENT_NONE;
	evtime->fd[0]=fd[0];
	evtime->fd[1]=fd[1];
	evtime->count=0;
	evtime->sec=sec;
	evtime->msec=msec;
	evtime->timeProc=NULL;
	evtime->timeTrigger=NULL;
	evtime->clientData=NULL;
}


event_base *createEventTimeout(event_proxy *proxy,long long milliseconds){
	int sockfd[2];
	if(socketpair(AF_LOCAL,SOCK_STREAM,0,sockfd)==-1)
		return NULL;
	int yes=1;
	if(setsockopt(sockfd[0],SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes))==-1){
		goto error;
	}
	if(setsockopt(sockfd[1],SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes))==-1){
		goto error;
	}
	int fd=sockfd[0];
	if(fd>proxy->setsize){
		if(resizeProxySetsize(proxy,fd+DEFAULT_PROXY_INTERVAL)==EVENT_ERR){
			goto error;
		}
	}
	event_storage_manager *manager=proxy->ev_timeout_manager;
	event_base *ev=GetEventFromStorage(manager);
	if(ev==NULL){
		goto error;
	}
	long sec,msec;
	evAddMillisecondsToNow(milliseconds,&sec,&msec);
	initEventTimeout(ev,sockfd,sec,msec);
	ev->fd=fd;
	if(addTriggerProc(proxy,ev,NULL)==EVENT_ERR){
		UndoEventFromStorage(manager,ev);
		goto error;
	}
	proxy->events[fd]=ev;
	proxy->elements++;
	if(fd>proxy->maxfd)
		proxy->maxfd=fd;
	return ev;
error:
	close(sockfd[0]);
	close(sockfd[1]);
	return NULL;
}

int addTimeoutEvent(event_proxy *proxy,event_base *ev,evTimeProc *timeProc,void *clientData){
	if(!proxy || !ev || ev->type!=EVENT_TIMEOUT){
		return EVENT_ERR;
	}
	if(ev->mask==EVENT_NONE){
		if(addPollEvent(proxy,ev->fd,EVENT_READABLE)==EVENT_ERR){
			return EVENT_ERR;
		}
		ev->mask=EVENT_READABLE;
	}
	event_timeout *evtime=(event_timeout*)ev;
	if(timeProc!=NULL)
		evtime->timeProc=timeProc;
	evtime->clientData=clientData;
	return EVENT_OK;
}

long long getEventTime(event_base *ev){
	if(!ev || ev->type!=EVENT_TIMEOUT)
		return 0;
	event_timeout *evtime=(event_timeout*)ev;
	long long ms;
	ms=evtime->sec*1000+evtime->msec;
	return ms;
}

long long getEventTimeout(event_base *ev){
	if(!ev || ev->type!=EVENT_TIMEOUT)
		return 0;
	long long cur_ms=evGetMillisecondsNow();
	long long timeout=getEventTime(ev);
	long long ms=timeout-cur_ms;
	return ms;
}

void addMillisecondsToEventTimeout(event_base *ev,long long ms){
	if(!ev || ev->type!=EVENT_TIMEOUT)return;
	event_timeout *evtime=(event_timeout*)ev;
	int ops=0;
	if(ms<0){
		ms=-ms;
		ops=1;
	}
	long sec=ms/1000;
	long msec=ms%1000;
	evtime->sec=evtime->sec+sec;
	evtime->msec=evtime->msec+msec;
	ev->flag=EVENT_ACTIVE | EVENT_TRIGGERABLE;
}

void setMillisecondsToEventTimeout(event_base *ev,long long ms){
	if(!ev || ev->type!=EVENT_TIMEOUT)return;
	event_timeout *evtime=(event_timeout*)ev;
	long sec=ms/1000;
	long msec=ms%1000;
	evtime->sec=sec;
	evtime->msec=msec;
	ev->flag=EVENT_ACTIVE | EVENT_TRIGGERABLE;
}

void evGenericTimeoutProc(event_proxy *proxy,event_base *ev,void *clientData){
	if(readFromEventWithNonBlock(ev,1)==1){
		event_timeout *evtime=(event_timeout*)ev;
		if(evtime->timeProc)
			evtime->timeProc(proxy,ev,clientData);
		ev->flag=ev->flag & ~EVENT_ACTIVE;
		ev->flag=ev->flag | EVENT_STOP;
	}
}

void evGenericTimeTriggerProc(event_proxy *proxy,event_base *ev,void *clientData){
	int triggerable=ev->flag & EVENT_TRIGGERABLE;
	if(!triggerable)return;
	long long ms=getEventTimeout(ev);
	if(ms<=0){
		if(trigger(ev)==EVENT_ERR)
			return;
		((event_timeout*)ev)->count++;
		ev->flag=ev->flag & ~EVENT_TRIGGERABLE;
	}
}

void freeEventTimeout(struct event_proxy *proxy,struct event_base *ev){
	if(!proxy || !ev || ev->type!=EVENT_TIMEOUT)
		return;
	event_timeout *evtime=(event_timeout*)ev;
	close(evtime->fd[0]);
	close(evtime->fd[1]);
}
