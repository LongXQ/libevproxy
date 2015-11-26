#include "event_user.h"
#include "event.h"
#include "event_epoll.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>



int getUserEventInReady(event_base *ev){
	if(!ev || ev->type!=EVENT_USER)
		return EVENT_ERR;
	event_user *user=(event_user*)ev;
	return user->count-user->done;
}

void initEventUser(event_base *ev,int sockfd[2],int maxtimes,evUserTriggerProc *triggerProc){
	if(!ev)return;
	event_user *user=(event_user*)ev;
	user->type=EVENT_USER;
	user->flag=EVENT_TRIGGERABLE | EVENT_ACTIVE;
	user->mask=EVENT_NONE;
	user->fd[0]=sockfd[0];
	user->fd[1]=sockfd[1];
	user->count=0;
	user->done=0;
	user->max=maxtimes;
	user->userProc=NULL;
	user->cleanProc=NULL;
	user->userTrigger=triggerProc;
	user->clientData=NULL;
}

event_base *createEventUser(event_proxy *proxy,int maxtimes,evUserTriggerProc *triggerProc,void *clientData){
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
	initEventUser(ev,sockfd,maxtimes,triggerProc);
	ev->fd=fd;
	if(addTriggerProc(proxy,ev,clientData)==EVENT_ERR){
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

int addUserEvent(struct event_proxy *proxy,struct event_base *ev,evUserProc *userProc,evCleanAfterUserProc *cleanProc,void *clientData){
	if(!proxy || !ev || ev->type!=EVENT_USER){
		return EVENT_ERR;
	}
	if(ev->mask == EVENT_NONE){
		if(addPollEvent(proxy,ev->fd,EVENT_READABLE)==EVENT_ERR){
			return EVENT_ERR;
		}
		ev->mask=EVENT_READABLE;
	}
	event_user *user=(event_user*)ev;
	if(userProc!=NULL)
		user->userProc=userProc;
	if(cleanProc!=NULL)
		user->cleanProc=cleanProc;
	user->clientData=clientData;
	return EVENT_OK;
}

//只有在处于EVENT_ACTIVE状态时才会执行这个函数
void  evGenericUserProc(struct event_proxy *proxy,struct event_base *ev,void *clientData){
	if(readFromEventWithNonBlock(ev,1)==1){
		event_user *user=(event_user*)ev;
		user->done++;
		if(user->userProc)
			user->userProc(proxy,ev,clientData);
		if(user->cleanProc)
			user->cleanProc(proxy,ev,clientData);
	}
}

//只有在处于EVENT_ACTIVE状态时才会执行这个函数
void evGenericUserTriggerProc(struct event_proxy *proxy,struct event_base *ev,void *clientData){
	int triggerable=ev->flag & EVENT_TRIGGERABLE;
	if(!triggerable)return;
	event_user *user=(event_user *)ev;
	if(user->userTrigger==NULL)return;
	if(user->userTrigger(proxy,ev,clientData)==NEED_TRIGGER){
		if(trigger(ev)==EVENT_ERR)return;
		user->count++;
		if(user->count>=user->max){
			user->flag=user->flag & ~EVENT_TRIGGERABLE;
		}
	}
}