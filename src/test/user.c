#include "../event.h"
#include "../event_user.h"
#include "../event_epoll.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


typedef struct user{
		int a;
}user;

int userTriggerProc(event_proxy *proxy,event_base *ev,void *clientData){
		user *us=(user*)clientData;
		if(us->a==10){
				return NEED_TRIGGER;
		}
		us->a++;
		return DONT_TRIGGER;
}

void userProc(event_proxy *proxy,event_base *ev,void *clientData){
		user *us=(user*)clientData;
		printf("a = %d\n",us->a);
}

void userCleanProc(event_proxy *proxy,event_base *ev,void *clientData){
		user *us=(user*)clientData;
		us->a=0;
}

int main(){
		user u;
		u.a=0;
		event_proxy *proxy=createEventProxy(1024);
		if(proxy==NULL){
				printf("proxy == NULL\n");
				return 0;
		}
		event_base *event=createEventUser(proxy,2,userTriggerProc,(void*)&u);
		if(event==NULL){
				printf("event == NULL\n");
				return 0;
		}
		event_user *user=(event_user*)event;
		printf("fd[0] = %d\n",user->fd[0]);
		printf("fd[1]= %d\n",user->fd[1]);
		int rv=addUserEvent(proxy,event,userProc,userCleanProc,(void*)&u);
		if(rv==EVENT_ERR)
				printf("rv == EVENT_ERR\n");
		while(1){
				eventLoop(proxy);
		}

		return 0;
}
