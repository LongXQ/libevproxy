#include "../event.h"
#include "../event_file.h"
#include "../event_timeout.h"
#include "../event_user.h"
#include "../event_signal.h"
#include "../util.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>

void writeProc(int fd,void *clientData){
	char buf[]="hello,world!";
	write(fd,buf,strlen(buf));
}

void readProc(int fd,void *clientData){
	char buf[10];
	read(fd,buf,10);
	printf("readProc ok\n");
}

void timeoutProc(event_proxy *proxy,event_base *ev,void*clientData){
		printf("超时发生了---------------------------------------------!\n");
}

typedef struct user{
		int a;
}user;

int userTriggerProc(event_proxy *proxy,event_base *ev,void *clientData){
		user *us=(user*)clientData;
		if(us->a==10000){
				return NEED_TRIGGER;
		}
		us->a++;
		return DONT_TRIGGER;
}

void userProc(event_proxy *proxy,event_base *ev,void *clientData){
		user *us=(user*)clientData;
		printf("----------------------a = %d-------------------------------\n",us->a);
}

void userCleanProc(event_proxy *proxy,event_base *ev,void *clientData){
		user *us=(user*)clientData;
		us->a=0;
}

void signalProc(event_proxy *proxy,event_base *ev,int signo,void *clientData){
		printf("the signal number is %d\n",signo);
}


int main(int argc,char**argv){
	int rv;
	user u;
	u.a=0;
	event_proxy *proxy=createEventProxy(1024);
	if(proxy==NULL){
		printf("createEventProxy error\n");
		return -1;
	}

	int port=6379;
	int number=10000;
	while(number){		
		int fd=socket(AF_INET,SOCK_DGRAM,0);
		struct sockaddr_in sin;
		sin.sin_addr.s_addr=INADDR_ANY;
		sin.sin_family=AF_INET;
		sin.sin_port=htons(port++);
		bind(fd,(struct sockaddr*)&sin,sizeof(sin));
		event_base *ev_file=createEventFile(proxy,fd);
		if(ev_file==NULL){
			printf("createEventFile error\n");
			return -1;
		}

		rv=addFileEvent(proxy,ev_file,EVENT_READABLE,readProc,NULL);
			if(rv==EVENT_ERR){
			printf("addFileEvent error\n");
			return -1;
		}
		rv=addFileEvent(proxy,ev_file,EVENT_WRITEABLE,writeProc,NULL);
		if(rv==EVENT_ERR){
			printf("addFileEvent error\n");
			return -1;
		}

		number--;
	}


	event_base *ev_timeout=createEventTimeout(proxy,5000);
	if(ev_timeout==NULL){
		printf("createEventTimeout error\n");
		return -1;
	}
	rv=addTimeoutEvent(proxy,ev_timeout,timeoutProc,NULL);
	if(rv==EVENT_ERR){
		printf("addTimeoutEvent error\n");
		return -1;
	}

	event_base *ev_user=createEventUser(proxy,2,userTriggerProc,(void*)&u);
	if(ev_user==NULL){
		printf("createEventUser error\n");
		return -1;
	}
	rv=addUserEvent(proxy,ev_user,userProc,userCleanProc,(void*)&u);
	if(rv==EVENT_ERR){
		printf("addUserEvent error\n");
		return -1;
	}

	sigset_t set;
	sigemptyset(&set);
	event_base *ev_signal=createEventSignal(proxy,SIGCHLD,set);
	if(ev_signal==NULL){
		printf("createEventSignal error\n");
		return -1;
	}
	rv=addSignalEvent(proxy,ev_signal,signalProc,NULL);
	if(rv==EVENT_ERR){
		printf("addSignalEvent error\n");
		return -1;
	}

	if(fork()==0){
		//child process
		exit(0);
	}

	printf("eventLoop start\n");

	while(1){
		eventLoop(proxy);
	}

	return 0;
}
