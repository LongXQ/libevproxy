#ifndef EVENT_FILE_HEADER
#define EVENT_FILE_HEADER


typedef void evFileProc(int fd,void *clientData);


typedef struct event_file{
	int type;
	/* 值可以为标记这个属性是否是一个需要触发的事件类型。
	因为有些事件是不需要自己调用触发函数来检查是否需要触发对象发生的，比如文件可读可写，
	文件的可读可写事件是由内核触发的，不需要自己来实现触发函数。
	而对于时间超时时间，则要用户自己定义触发函数来检查时间是否超时了，如果超时了则触发这个事件，
	在比如用户自定义的事件，往往需要定义触发函数来检查自定义的事件是否需要触发了，因为内核不清楚这些事件该什么时候出发，
	而一般的和文件I/O相关的事件，由于文件系统是由内核自己实现的，所以它知道该什么时候出发文件事件的发送，然后epoll就可以检查到事件是否发生了 */
	int flag; //EVENT_TRIGGER,EVENT_ACTIVE,EVENT_STOP,EVENT_NONE
	int mask; //EVENT_READABLE,EVENT_WRITEABLE
	int id; //id是和storage有关的属性
	int fd; //这个fd是readproc和writeProc要操作的文件描述符
	evFileProc *rfileProc;
	evFileProc *wfileProc;
	void *clientData;
}event_file;


struct event_base;
struct event_proxy;

//createEventFile是一个全局函数
struct event_base *createEventFile(struct event_proxy *proxy,int fd);
int addFileEvent(struct event_proxy *proxy,struct event_base *ev,int mask,evFileProc *fileProc,void *clientData);
void initEventFile(struct event_base *ev);
#endif
