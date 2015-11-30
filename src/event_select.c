#include "event_select.h"
#include "event.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>

int createEventPoll(event_proxy *proxy,int setsize){
	event_poll *ev_poll=(event_poll*)malloc(sizeof(event_poll));
	if(!ev_poll)return EVENT_ERR;
	FD_ZERO(&ev_poll->rfds);
	FD_ZERO(&ev_poll->wfds);
	proxy->ev_poll=ev_poll;
	return EVENT_OK;
}

int resizeEventPoll(event_proxy *proxy,int setsize){
	if(setsize>FD_SETSIZE)return EVENT_ERR;
	return EVENT_OK;
}

void deleteEventPoll(event_proxy *proxy){
	event_poll *ev_poll=proxy->ev_poll;
	free(ev_poll);
}

int addPollEvent(event_proxy *proxy,int fd,int mask){
	event_poll *ev_poll=proxy->ev_poll;
	if (mask & EVENT_READABLE) FD_SET(fd,&ev_poll->rfds);
    if (mask & EVENT_WRITEABLE) FD_SET(fd,&ev_poll->wfds);
    return EVENT_OK;
}

void delPollEvent(event_proxy *proxy,int fd,int delmask){
	event_poll *ev_poll=proxy->ev_poll;
	if (mask & EVENT_READABLE) FD_CLR(fd,&ev_poll->rfds);
    if (mask & EVENT_WRITEABLE) FD_CLR(fd,&ev_poll->wfds);
}

int eventPoll(event_proxy *proxy,struct timeval *tvp){
	event_poll *ev_poll=proxy->ev_poll;
	int retval,numevents=0;

	memcpy(&ev_poll->_rfds,&ev_poll->rfds,sizeof(fd_set));
	memcpy(&ev_poll->_wfds,&ev_poll->wfds,sizeof(fd_set));

	retval=select(proxy->maxfd+1,&ev_poll->_rfds,&ev_poll->_wfds,NULL,tvp);

	if(retval>0){
		int j=0;
		for(j=0;j<proxy->maxfd;j++){
			int mask=0;
			event_base *event=proxy->events[j];
			if(event->mask==EVENT_NONE)continue;
			if(event->mask & EVENT_READABLE && FD_ISSET(j,&ev_poll->_rfds))
				mask |= EVENT_READABLE;
			if (event->mask & EVENT_WRITEABLE && FD_ISSET(j,&ev_poll->_wfds))
                mask |= EVENT_WRITEABLE;
            proxy->ev_fires[numevents].fd=j;
			proxy->ev_fires[numevents].mask=mask;
			numevents++;
		}
	}
	return numevents;
}

char *eventPollName(void){
	return "select";
}
