//virtual event system
//所有的事件都统一与文件描述符中，这样就可以统一使用epoll来判断是否事件发生了
//每一个具体的事件对象的readProc和writeProc是每一个事件的定义的具体的处理函数，当然也可以不实现这两个函数，那么久使用事件代理实现的通用的处理函数
#ifndef EVENT_HEADER
#define EVENT_HEADER
#define EV_READABLE 0x01
#define EV_WRITEABLE 0x02
#define EV_NEEDTRIGGER 0X10;


//因为每一个事件的operation都不相同，所以，为了每一种事件都定义一系列操作
typedef struct event_proxy_operations{
	event_base *addEvent(int type,...); //由于是变参函数，所以参数根据type不同而不同
	removeEvent(event_base *);
	eventloop();
}event_proxy_operations;

typedef struct event_proxy{
	int setsize;
	event_proxy_base *ev_proxy;
	event_poll *ev_poll;
	event_base **ev_triger; //需要触发的事件，因为并不是每一个事件需要触发，所以为了减少遍历的时间，增加了这么一个成员
	event_base **events; //注册到epoll中的不同类型的所有事件
	event_fired *ev_fires; //就绪的事件列表，因为每一个事件都和你一个文件描述符相关联
}event_proxy;


//下面这个event_poll是针对epoll的
typedef struct event_poll {
	int fd;
	struct epoll_event *events;
}event_poll;

typedef struct event_fired {
	int fd;
	int mask; //READABLE || WRITEABLE
}event_fired;

int __trigger(event_base *ev); //这个函数是内部支持的触发函数，提供给用户定义的触发函数调用

#endif