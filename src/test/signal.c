#include "../event.h"
#include "../event_signal.h"
#include "../event_epoll.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void signalProc(event_proxy *proxy,event_base *ev,int signo,void *clientData){
		printf("the signal number is %d\n",signo);
}

int main(){
		event_proxy *proxy=createEventProxy(1024);
		if(proxy==NULL){
				printf("proxy == NULL\n");
				return 0;
		}
		sigset_t set;
		sigemptyset(&set);
		event_base *event=createEventSignal(proxy,SIGCHLD,set);
		if(event==NULL){
				printf("event == NULL\n");
				return 0;
		}
		event_signal *sig=(event_signal*)event;
		int rv=addSignalEvent(proxy,event,signalProc,NULL);
		if(rv==EVENT_ERR)
				printf("rv == error\n");
		if(fork()==0){
				//child process
				exit(0);
		}else{
				while(1){
						eventLoop(proxy);
				}
		}
		return 0;
}
