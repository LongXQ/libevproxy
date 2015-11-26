#include "../event_timeout.h"
#include "../event.h"
#include <stdio.h>
#include <time.h>

void proc(event_proxy *proxy,event_base *ev,void*clientData){
		printf("超时发生了!\n");
}

int main(){
		event_proxy *proxy=createEventProxy(6048);
		if(proxy==NULL)
				printf("proxy == NULL\n");
		event_base *event=createEventTimeout(proxy,5000);
		event_base *event1=createEventTimeout(proxy,10000);
		if(event==NULL)
				printf("event == NULL\n");
		event_timeout *timeout=(event_timeout*)event;
		printf("event->fd[0] = %d\n",timeout->fd[0]);
		printf("event->fd[1] = %d\n",timeout->fd[1]);
		int rv=addTimeoutEvent(proxy,event,proc,NULL);
		addTimeoutEvent(proxy,event1,proc,NULL);
		if(rv==EVENT_ERR){
				printf("addTimeoutEvent error\n");
				return 0;
		}
		while(1){
			eventLoop(proxy);
		}
		return 0;
}
