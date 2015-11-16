#include "event_interface.h"
#ifndef EVENT_FILE_HEADER
#define EVENT_FILE_HEADER

#define void fileReadProc(int fd,void *clientData)
#define void fileWriteProc(int fd,void *clientData)
#define void fileTriggerProc(int fd)
#define int removeFileEvent(event_base *)
#define void triggerFileEvent(event_base *);

typedef struct event_file_proxy_operations{
	removeFileEvent removeEvent;
	triggerFileEvent triggerEvent;
}event_file_proxy_operations;

typedef struct event_file_operations{
	fileReadProc read;
	fileWriteProc write;
	fileTriggerProc trigger;
}event_file_operations;

typedef struct event_file{
	int type;
	int fd; //这个fd是readproc和writeProc要操作的文件描述符
	/* 也许可以和type字段合并成一个字段 值可以为标记这个属性是否是一个需要触发的事件类型。
	因为有些事件是不需要自己调用触发函数来检查是否需要触发对象发生的，比如文件可读可写，
	文件的可读可写事件是由内核触发的，不需要自己来实现触发函数。
	而对于时间超时时间，则要用户自己定义触发函数来检查时间是否超时了，如果超时了则触发这个事件，
	在比如用户自定义的事件，往往需要定义触发函数来检查自定义的事件是否需要触发了，因为内核不清楚这些事件该什么时候出发，
	而一般的和文件I/O相关的事件，由于文件系统是由内核自己实现的，所以它知道该什么时候出发文件事件的发送，然后epoll就可以检查到事件是否发生了 */
	int id;
	int flag; //EV_TRIGGER,EV_ACTIVE,EV_STOP,EV_
	int mask;
	event_file_operations *ops;
	void *clientData
}event_file;

typedef struct event_file_proxy{
	int type;
	int setsize;
	int elements;
	int next_id; 
	event_file *events; //这个fd对应的是具体的事件对象；
	event_proxy_operations *ops;
}event_file_proxy;

int _freeFileEvent();
int _removeFileEvent(event_base *ev);
void _triggerFileEvent(event_base *ev);
//这个函数是暴露给外部的,每一个事件都要暴露这个函数给外部使用
event_base *createFileEvent(event_proxy_base *ev_proxy,int fd,int mask,fileReadProc ev_read,fileWriteProc ev_write,void *clientData);
#endif