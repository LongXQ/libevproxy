#ifndef EVENT_SIGNAL_HEADER
#define EVENT_SIGNAL_HEADER

#include <signal.h>

struct event_base;
struct event_proxy;

typedef void evSignalProc(struct event_proxy*,struct event_base *,int signo,void *clientData);

typedef struct event_signal {
	int type;
	int flag;
	int mask; //timeout事件永远是可读的事件
	int id;
	int fd[2];
	int count; //记录被事件发生的次数
	int done;
	int signo;
	sigset_t sigset; //在事件处理函数运行的时候需要阻塞的信号
	struct sigaction oact;
	evSignalProc *signalProc;
	void *clientData;
}event_signal;

void initEventSignal(struct event_base *ev,int sockfd[2],int signo,sigset_t sigset);
void evGenericSignalHandler(int signo);
struct event_base *createEventSignal(struct event_proxy *proxy,int signo,sigset_t sigset);
void freeEventSignal(struct event_base *ev);
void setBlockSignal(struct event_base *ev,sigset_t sigset);
int addSignalEvent(struct event_proxy *proxy,struct event_base *ev,evSignalProc *signalProc,void *clientData);
void evGenericSignalProc(struct event_proxy *proxy,struct event_base *ev,int signo,void *clientData);
void ignoreSignal(struct event_base *ev);
void activeSignal(struct event_base *ev);

#endif