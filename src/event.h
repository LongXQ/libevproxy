//所有的事件都统一与文件描述符中，这样就可以统一使用epoll来判断是否事件发生了
//每一个具体的事件对象的readProc和writeProc是每一个事件的定义的具体的处理函数，当然也可以不实现这两个函数，那么久使用事件代理实现的通用的处理函数
#ifndef EVENT_HEADER
#define EVENT_HEADER

#include "event_epoll.h"

#define EVENT_NONE 0x00

#define EVENT_FILE 0X01
#define EVENT_TIMEOUT 0x02
#define EVENT_SIGNAL 0x04
#define EVENT_USER 0x08

#define EVENT_READABLE 0x01
#define EVENT_WRITEABLE 0x02

#define EVENT_TRIGGERABLE 0x10
#define EVENT_ACTIVE 0x01
#define EVENT_STOP 0x02

#define EVENT_OK 0
#define EVENT_ERR -1

#define STORAGE_NULL 0x00
#define STORAGE_IN 0x01
#define STORAGE_FULL 0x02

#define STORAGE_BLOCKS 1
#define STORAGE_SETSIZE 1024

#define STORAGE_FILE_BLOCKS 1
#define STORAGE_FILE_SETSIZE 1024
#define STORAGE_TIMEOUT_BLOCKS 1
#define STORAGE_TIMEOUT_SETSIZE 64
#define STORAGE_SIGNAL_BLOCKS 1
#define STORAGE_SIGNAL_SETSIZE 64
#define STORAGE_USER_BLOCKS 1
#define STORAGE_USER_SETSIZE 64

#define DEFAULT_PROXY_SETSIZE 1024
#define DEFAULT_PROXY_INTERVAL 1024


typedef struct event_base{
	int type;
	int flag; //EVENT_TRIGGERABLE
	int mask;
	int id;
	int fd;
	char data[8];
}event_base;

struct event_storage;

typedef struct event_storage_header{
	int setsize;
	int elements;
	int status;
	int next_pos;
	struct event_storage *next;
}event_storage_header;

typedef struct event_storage{
	event_storage_header header;
	event_base events[0];
}event_storage;

typedef struct event_storage_manager{
	int type;
	int blocks;
	int free_blocks;
	event_storage *current;
	event_storage *head;
	event_storage *tail;
}event_storage_manager;

typedef struct event_trigger{
	event_base *event;
	void *clientData;
	struct event_trigger *next;
}event_trigger;

typedef struct event_fired {
		int fd;
		int mask; //READABLE || WRITEABLE
}event_fired;

typedef struct event_proxy{
	//此处的setsize和elements指的是ev_poll中和events和ev_fires的大小
	int setsize;
	int elements;
	int maxfd;

	event_storage_manager *ev_file_manager;
	event_storage_manager *ev_timeout_manager;
	event_storage_manager *ev_signal_manager;
	event_storage_manager *ev_user_manager;

	event_poll *ev_poll;
	event_trigger *ev_triggerfunc; //需要触发的事件，因为并不是每一个事件需要触发，所以为了减少遍历的时间，增加了这么一个成员
	event_base **events;
	event_fired *ev_fires; //就绪的事件列表，因为每一个事件都和你一个文件描述符相关联
}event_proxy;

typedef void evTriggerProc(event_proxy *proxy,event_base *ev,void *clientData);

int trigger(event_base *ev); //这个函数是内部支持的触发函数，提供给用户定义的触发函数调用
int evGenericProc(event_base *);
event_base *GetEventFromStorage(event_storage_manager *manager);
void UndoEventFromStorage(event_storage_manager *manager,event_base *ev);
event_proxy *createEventProxy(int setsize);
int resizeProxySetsize(event_proxy *proxy,int setsize);
void deleteEventProxy(event_proxy *proxy);
void delEvent(event_proxy *proxy,event_base *ev,int delmask);
void freeEvent(event_proxy *proxy,event_base *ev);
void evStopEvent(event_base *ev);
void evActiveEvent(event_base *ev);
void eventLoop(event_proxy *);
int addTriggerProc(event_proxy *,event_base *,void *clientData);
void evEnableTrigger(event_base *ev);
void evDisableTrigger(event_base *ev);
int readFromEventWithNonBlock(event_base *ev,int n);
#endif
