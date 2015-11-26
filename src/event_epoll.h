#ifndef EVENT_EPOLL_HEADER
#define EVENT_EPOLL_HEADER
#include <sys/time.h>
#include <sys/epoll.h>

//下面这个event_poll是针对epoll的
typedef struct event_poll {
	int epfd;
	struct epoll_event *events;
}event_poll;

struct event_proxy;
int createEventPoll(struct event_proxy*,int);
int resizeEventPoll(struct event_proxy *,int);
void deleteEventPoll(struct event_proxy *);
int addPollEvent(struct event_proxy *,int,int);
void delPollEvent(struct event_proxy *,int,int);
int eventPoll(struct event_proxy *,struct timeval *);
char *eventPollName(void);
#endif
