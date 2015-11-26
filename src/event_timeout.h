#ifndef EVENT_TIMEOUT_HEADER
#define EVENT_TIMEOUT_HEADER

struct event_base;
struct event_proxy;
extern int trigger(struct event_base *ev);

typedef void evTimeProc(struct event_proxy *proxy,struct event_base *ev,void *clientData);
typedef void evTimeTriggerProc(struct event_base *ev,void *clientData);

typedef struct event_timeout {
	int type;
	int flag;
	int mask; //timeout事件永远是可读的事件
	int id;
	int fd[2];
	int count; //记录发生的次数
	long sec;
	long msec;
	evTimeProc *timeProc;
	evTimeTriggerProc *timeTrigger;
	void *clientData;
}event_timeout;



struct event_base *createEventTimeout(struct event_proxy *proxy,long long milliseconds);
void freeEventTimeout(struct event_proxy *proxy,struct event_base *ev);
int addTimeoutEvent(struct event_proxy *proxy,struct event_base *ev,evTimeProc *timeProc,void *clientData);
void evGenericTimeTriggerProc(struct event_proxy *proxy,struct event_base *ev,void *clientData);
void evGenericTimeoutProc(struct event_proxy *proxy,struct event_base *ev,void *clientData);
long long getEventTime(struct event_base *);//获取timeout事件的超时设置时间
long long getEventTimeout(struct event_base *); //得到还有多少时间事件发生
void addMillisecondsToEventTimeout(struct event_base *ev,long long);
void setMillisecondsToEventTimeout(struct event_base *ev,long long ms);



#endif
