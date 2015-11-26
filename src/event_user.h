#ifndef EVENT_USER_HEADER
#define EVENT_USER_HEADER

#define DONT_TRIGGER 0x00
#define NEED_TRIGGER 0x01
struct event_base;
struct event_proxy;
extern int trigger(struct event_base *ev);

/* 以下两个函数是用户自己定义的函数，而对于用户来说，
 * 整个事件框架可见的只有event_proxy,event_base记忆操作这两个结构的一系列操作函数而已，
 * 所以在每个用户定义的函数的参数应该都包含event_proxy和event_base这两个参数
 */
typedef void evUserProc(struct event_proxy *proxy,struct event_base *ev,void *clientData);
typedef int evUserTriggerProc(struct event_proxy *proxy,struct event_base *ev,void *clientData);
typedef void evCleanAfterUserProc(struct event_proxy *proxy,struct event_base *ev,void *clientData);



typedef struct event_user {
	int type;
	int flag;
	int mask; //user事件永远是可读的事件
	int id;
	int fd[2];
	int count; //记录发生的次数
	int done; //记录已经处理的事件
	int max; //最多发生多少次,-1表示无限。
	evUserProc *userProc;
	evCleanAfterUserProc *cleanProc; 
	evUserTriggerProc *userTrigger;
	void *clientData;
}event_user;


void initEventUser(struct event_base *ev,int sockfd[2],int maxtimes,evUserTriggerProc *triggerProc);
int getUserEventInReady(struct event_base *ev);
struct event_base *createEventUser(struct event_proxy *proxy,int maxtimes,evUserTriggerProc *triggerProc,void *clientData);
int addUserEvent(struct event_proxy *proxy,struct event_base *ev,evUserProc *userProc,evCleanAfterUserProc *cleanProc,void *clientData);
void evGenericUserTriggerProc(struct event_proxy *proxy,struct event_base *ev,void *clientData);
void  evGenericUserProc(struct event_proxy *proxy,struct event_base *ev,void *clientData);

#endif
