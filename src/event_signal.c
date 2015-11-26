#include "event_signal.h"
#include "event.h"
#include "event_epoll.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>

event_base* sigPtr[sizeof(sigset_t)*8]={NULL};

void initEventSignal(event_base *ev,int sockfd[2],int signo,sigset_t sigset){
	if(!ev)return;
	event_signal *evsignal=(event_signal*)ev;
	evsignal->type=EVENT_SIGNAL;
	evsignal->flag=EVENT_TRIGGERABLE | EVENT_ACTIVE;
	evsignal->mask=EVENT_NONE;
	evsignal->fd[0]=sockfd[0];
	evsignal->fd[1]=sockfd[1];
	evsignal->count=0;
	evsignal->done=0;
	evsignal->signo=signo;
	evsignal->sigset=sigset;
	evsignal->signalProc=NULL;
	evsignal->clientData=NULL;
}

void evGenericSignalHandler(int signo){
	event_base *ev=sigPtr[signo];
	event_signal *evsignal=(event_signal*)ev;
	if(ev && (ev->flag & EVENT_TRIGGERABLE)){
		if(trigger(ev)==EVENT_ERR)return;
		evsignal->count++;
	}
}

event_base *createEventSignal(event_proxy *proxy,int signo,sigset_t sigset){
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
	event_storage_manager *manager=proxy->ev_user_manager;
	event_base *ev=GetEventFromStorage(manager);
	if(ev==NULL){
		goto error;
	}
	initEventSignal(ev,sockfd,signo,sigset);
	ev->fd=fd;
	proxy->events[fd]=ev;
	sigPtr[signo]=ev;
	struct sigaction act;
	act.sa_handler=evGenericSignalHandler;
	sigemptyset(&act.sa_mask);
	act.sa_flags=SA_RESTART;
	sigaction(signo,&act,&((event_signal*)ev)->oact);
	proxy->elements++;
	if(fd>proxy->maxfd)
		proxy->maxfd=fd;
	return ev;
error:
	close(sockfd[0]);
	close(sockfd[1]);
	return NULL;	
}

void freeEventSignal(event_base *ev){
	event_signal *evsignal=(event_signal*)ev;
	sigPtr[evsignal->signo]=NULL;
	sigaction(evsignal->signo,&evsignal->oact,NULL);
	close(evsignal->fd[0]);
	close(evsignal->fd[1]);
}

void setBlockSignal(event_base *ev,sigset_t sigset){
	if(!ev || ev->type!=EVENT_SIGNAL)return;
	event_signal *evsignal=(event_signal*)ev;
	evsignal->sigset=sigset;
}

int addSignalEvent(event_proxy *proxy,event_base *ev,evSignalProc *signalProc,void *clientData){
	if(!proxy || !ev || ev->type!=EVENT_SIGNAL)
		return EVENT_ERR;
	if(ev->mask ==EVENT_NONE){
		if(addPollEvent(proxy,ev->fd,EVENT_READABLE)==EVENT_ERR)
			return EVENT_ERR;
		ev->mask = EVENT_READABLE;
	}
	event_signal *evsignal=(event_signal*)ev;
	if(signalProc!=NULL)
		evsignal->signalProc=signalProc;
	evsignal->clientData=clientData;
	return EVENT_OK;
}

void evGenericSignalProc(event_proxy *proxy,event_base *ev,int signo,void *clientData){
	event_signal *evsignal=(event_signal*)ev;
	if(readFromEventWithNonBlock(ev,1)==1){
		sigset_t set=evsignal->sigset;
		sigset_t oset;
		sigprocmask(SIG_BLOCK,&set,&oset);
		if(evsignal->signalProc)
			evsignal->signalProc(proxy,ev,signo,clientData);
		evsignal->done++;
		sigprocmask(SIG_SETMASK,&oset,NULL);
	}
}


void ignoreSignal(event_base *ev){
	ev->flag = ev->flag & ~EVENT_TRIGGERABLE;
}

void activeSignal(event_base *ev){
	ev->flag = ev->flag | EVENT_TRIGGERABLE;
}

