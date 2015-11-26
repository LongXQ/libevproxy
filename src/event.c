#include "event_signal.h"
#include "event_timeout.h"
#include "event_user.h"
#include "event_epoll.h"
#include "event_file.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#include "event.h"



static void _initBlocks(int interval,event_storage *storage){
	int setsize=storage->header.setsize;
	event_base *prev=storage->events;
	for(int i=1;i<setsize;i++){
		prev->id=i;
		prev=(event_base*)((char*)prev+interval);
	}
	prev->id=-1;
}

event_storage *createStorage(int type,int setsize){
	int interval=0;
	if(type==EVENT_FILE)
		interval=sizeof(event_file);
	if(type==EVENT_SIGNAL)
		interval=sizeof(event_signal);
	if(type==EVENT_TIMEOUT)
		interval=sizeof(event_timeout);
	if(type==EVENT_USER)
		interval=sizeof(event_user);
	if(setsize<=0 || interval==0)
		return NULL;
	int size=sizeof(event_storage_header)+interval*setsize;
	event_storage *storage;
	if((storage=(event_storage*)malloc(size))==NULL)
		return NULL;
	storage->header.setsize=setsize;
	storage->header.elements=0;
	storage->header.status=STORAGE_NULL;
	storage->header.next_pos=0;
	storage->header.next=NULL;
	_initBlocks(interval,storage);
	return storage;
}

event_storage_manager *createStorageManager(int type,int blocks,int setsize){
	event_storage_manager *manager;
	int num=0;
	if((manager=(event_storage_manager*)malloc(sizeof(event_storage_manager)))==NULL)
		return NULL;
	manager->type=type;
	manager->head=NULL;
	manager->tail=NULL;
	event_storage *storage;
	for(int i=0;i<blocks;i++){
		if((storage=createStorage(type,setsize))==NULL)
			continue;
		num++;
		storage->header.next=NULL;
		if(manager->head==NULL)
			manager->head=storage;
		if(manager->tail!=NULL)
			(manager->tail)->header.next=storage;
		manager->tail=storage;
	}
	manager->current=manager->head;
	manager->current->header.status=STORAGE_IN;
	manager->blocks=num;
	manager->free_blocks=(num==0?0:(num-1));
	return manager;
}

void deleteStorageManager(event_storage_manager *manager){
	event_storage *current;
	if(manager->blocks==0)return;
	while(manager->head){
		current=manager->head;
		manager->head=current->header.next;
		free(current);
	}
}

int _GetNextStorage(event_storage_manager *manager){
	event_storage *storage=manager->head;

	while(storage!=NULL){
		if(storage->header.status==STORAGE_IN || storage->header.status==STORAGE_NULL){
			manager->current=storage;
			break;
		}
		storage=storage->header.next;
	}

	if(storage==NULL)
			return EVENT_ERR;
	if(storage->header.status==STORAGE_NULL)
		manager->free_blocks=manager->free_blocks-1;
	if(manager->current->header.status==STORAGE_FULL)
		return EVENT_ERR;
	return EVENT_OK;
}

event_base *GetNext(event_storage_manager *manager){
	event_storage *storage;
	if(manager==NULL)
		return NULL;
	if(manager->head==NULL){
		if((storage=createStorage(manager->type,STORAGE_SETSIZE))==NULL){
			return NULL;
		}
		storage->header.next=NULL;
		manager->blocks=1;
		manager->head=storage;
		manager->tail=storage;
		manager->current=storage;
		manager->current->header.status=STORAGE_IN;
	}
	if(manager->current==NULL){
		if((storage=createStorage(manager->type,STORAGE_SETSIZE))==NULL){
			return NULL;
		}
		storage->header.next=NULL;
		manager->tail->header.next=storage;
		manager->tail=storage;
		manager->blocks=manager->blocks+1;
		manager->current=storage;
		manager->current->header.status=STORAGE_IN;
	}
	if(manager->current->header.status==STORAGE_FULL){
		if(_GetNextStorage(manager)==EVENT_ERR){
			if((storage=createStorage(manager->type,STORAGE_SETSIZE))==NULL){
				manager->current=NULL;
				return NULL;
			}
			manager->current=storage;
			manager->tail->header.next=storage;
			manager->tail=storage;
		}
	}
	//到此为止说明current中有空闲的空间
	storage=manager->current;
	event_base *ev=storage->events;
	int id=storage->header.next_pos;
	switch(manager->type){
		case EVENT_FILE:
			ev=(event_base*)&(((event_file*)ev)[id]);
			break;
		case EVENT_SIGNAL:
			ev=(event_base*)&(((event_signal*)ev)[id]);
			break;
		case EVENT_TIMEOUT:
			ev=(event_base*)&(((event_timeout*)ev)[id]);
			break;
		case EVENT_USER:
			ev=(event_base*)&(((event_user*)ev)[id]);
			break;
		default:
			return NULL;
	}
	storage->header.next_pos=ev->id;
	ev->id=id;
	storage->header.elements=storage->header.elements+1;
	if(storage->header.next_pos==-1)
		storage->header.status=STORAGE_FULL;
	return ev;
}

event_base *GetEventFromStorage(event_storage_manager *manager){
	return GetNext(manager);
}

event_storage *GetStorageFromID(event_base *ev){
	int id=ev->id;
	event_storage *storage;
	switch(ev->type){
		case  EVENT_FILE:
			storage=(event_storage*)(((char*)(((event_file*)ev)-id))-sizeof(event_storage));
			break;
		case EVENT_TIMEOUT:
			storage=(event_storage*)(((char*)(((event_timeout*)ev)-id))-sizeof(event_storage));
			break;
		case EVENT_SIGNAL:
			storage=(event_storage*)(((char*)(((event_signal*)ev)-id))-sizeof(event_storage));
			break;
		case EVENT_USER:
			storage=(event_storage*)(((char*)(((event_user*)ev)-id))-sizeof(event_storage));
			break;
		default:
			storage=NULL;
			break;
	}
	return storage;
}

void UndoEventFromStorage(event_storage_manager *manager,event_base *ev){
	event_storage *storage=GetStorageFromID(ev);
	int next_pos=storage->header.next_pos;
	storage->header.next_pos=ev->id;
	ev->id=next_pos;
	storage->header.elements--;
	if(storage->header.elements==0){
		storage->header.status=STORAGE_NULL;
		manager->free_blocks++;
	}
	if(storage->header.status==STORAGE_FULL)
		storage->header.status=STORAGE_IN;
	if(manager->current==NULL || manager->current->header.status==STORAGE_FULL)
		manager->current=storage;
}

void initEventProxy(event_proxy *proxy,int setsize){
	if(proxy==NULL)return;
	proxy->setsize=setsize;
	proxy->elements=0;
	proxy->maxfd=-1;
	proxy->ev_triggerfunc=NULL;

	proxy->ev_file_manager=NULL;
	proxy->ev_timeout_manager=NULL;
	proxy->ev_signal_manager=NULL;
	proxy->ev_user_manager=NULL;

	proxy->ev_poll=NULL;
	proxy->events=NULL;
	proxy->ev_fires=NULL;
}

event_proxy *createEventProxy(int setsize){
	event_proxy *proxy;
	if((proxy=(event_proxy*)malloc(sizeof(event_proxy)))==NULL)
		return NULL;
	initEventProxy(proxy,setsize);
	proxy->ev_file_manager=createStorageManager(EVENT_FILE,STORAGE_FILE_BLOCKS,STORAGE_FILE_SETSIZE);
	if(proxy->ev_file_manager==NULL)
		goto error;
	proxy->ev_timeout_manager=createStorageManager(EVENT_TIMEOUT,STORAGE_TIMEOUT_BLOCKS,STORAGE_TIMEOUT_SETSIZE);
	if(proxy->ev_timeout_manager==NULL)
		goto error;
	proxy->ev_signal_manager=createStorageManager(EVENT_SIGNAL,STORAGE_SIGNAL_BLOCKS,STORAGE_SIGNAL_SETSIZE);
	if(proxy->ev_signal_manager==NULL)
		goto error;
	proxy->ev_user_manager=createStorageManager(EVENT_USER,STORAGE_USER_BLOCKS,STORAGE_USER_SETSIZE);
	if(proxy->ev_user_manager==NULL)
		goto error;
	if(createEventPoll(proxy,setsize)==EVENT_ERR)
		goto error;
	proxy->events=(event_base**)malloc(sizeof(event_base*)*setsize);
	if(proxy->events==NULL)
		goto error;
	proxy->ev_fires=(event_fired*)malloc(sizeof(event_fired)*setsize);
	if(proxy->ev_fires==NULL)
		goto error;
	return proxy;
error:
	if(proxy->ev_file_manager!=NULL)
		free(proxy->ev_file_manager);
	if(proxy->ev_timeout_manager!=NULL)
		free(proxy->ev_timeout_manager);
	if(proxy->ev_signal_manager!=NULL)
		free(proxy->ev_signal_manager);
	if(proxy->ev_user_manager!=NULL)
		free(proxy->ev_user_manager);
	if(proxy->ev_poll!=NULL)
		free(proxy->ev_poll);
	if(proxy->events!=NULL)
		free(proxy->events);
	if(proxy->ev_fires!=NULL)
		free(proxy->ev_fires);
	free(proxy);
	return NULL;
}

int resizeProxySetsize(event_proxy *proxy,int setsize){
	if(setsize<=proxy->setsize)return EVENT_OK;
	if(resizeEventPoll(proxy,setsize)==EVENT_ERR)
		return EVENT_ERR;
	event_base **events;
	events=(event_base**)malloc(sizeof(event_base*)*setsize);
	//events=(event_base**)realloc(proxy->events,sizeof(event_base*)*setsize);
	if(events==NULL)
		return EVENT_ERR;
	memcpy((void*)events,(void*)proxy->events,proxy->setsize*sizeof(event_base*));
	free(proxy->events);
	proxy->events=events;
	event_fired *fired;
	fired=(event_fired*)malloc(sizeof(event_fired)*setsize);
	//fired=(event_fired*)realloc(proxy->ev_fires,sizeof(event_fired)*setsize);
	if(fired==NULL)
		return EVENT_ERR;
	memcpy((void*)fired,(void*)proxy->ev_fires,proxy->setsize*sizeof(event_fired));
	free(proxy->ev_fires);
	proxy->ev_fires=fired;
	proxy->setsize=setsize;
	return EVENT_OK;
}

void deleteEventProxy(event_proxy *proxy){
	deleteStorageManager(proxy->ev_file_manager);
	deleteStorageManager(proxy->ev_timeout_manager);
	deleteStorageManager(proxy->ev_signal_manager);
	deleteStorageManager(proxy->ev_user_manager);
	deleteEventPoll(proxy);
	event_trigger *current;
	while(proxy->ev_triggerfunc){
		current=proxy->ev_triggerfunc;
		proxy->ev_triggerfunc=current->next;
		free(current);
	}
	free(proxy->events);
	free(proxy->ev_fires);
	free(proxy);
}


void delEvent(event_proxy *proxy,event_base *ev,int delmask){
	if(ev==NULL)return;
	if(ev->mask==EVENT_NONE)return;
	delPollEvent(proxy,ev->fd,delmask);
	ev->mask=ev->mask & ~delmask;
	if (ev->fd == proxy->maxfd && ev->mask == EVENT_NONE) {
        /* Update the max fd */
        int j;

        for (j = proxy->maxfd-1; j >= 0; j--)
            if (proxy->events[j]->mask != EVENT_NONE) break;
        proxy->maxfd = j;
    }
}

void freeEvent(event_proxy *proxy,event_base *ev){
	if(ev==NULL)return;
	//删除事件
	delEvent(proxy,ev,ev->mask);
	switch(ev->type){
		case EVENT_FILE:
			UndoEventFromStorage(proxy->ev_file_manager,ev);
			break;
		case EVENT_TIMEOUT:
			UndoEventFromStorage(proxy->ev_timeout_manager,ev);
			break;
		case EVENT_SIGNAL:
			UndoEventFromStorage(proxy->ev_signal_manager,ev);
			break;
		case EVENT_USER:
			UndoEventFromStorage(proxy->ev_user_manager,ev);
			break;
		default:
			break;
	}	
}

void evStopEvent(event_base *ev){
	int trigger=ev->flag & EVENT_TRIGGERABLE;
	ev->flag = EVENT_STOP | trigger;
}

void evActiveEvent(event_base *ev){
	int trigger=ev->flag & EVENT_TRIGGERABLE;
	ev->flag=EVENT_ACTIVE | trigger;
}
 

int readFromEventWithNonBlock(event_base *ev,int n){
	if(n>0){
		char *buf=(char*)malloc(n+1);
		if(buf==NULL)
			return 0;
		int flags;
		int rv;
		if((flags=fcntl(ev->fd,F_GETFL))==-1)
			return 0;
		flags |= O_NONBLOCK;
		if(fcntl(ev->fd,F_SETFL,flags)==-1)
			return 0;
		if((rv=read(ev->fd,buf,n))<=0)
			return 0;
		return rv;
	}
	return 0;
}

int addTriggerProc(event_proxy *proxy,event_base *ev,void *clientData){
	if(!proxy || !ev)return EVENT_ERR;
	event_trigger *trigger=(event_trigger*)malloc(sizeof(event_trigger));
	if(trigger==NULL)return EVENT_ERR;
	trigger->event=ev;
	trigger->clientData=clientData;
	trigger->next=NULL;
	if(proxy->ev_triggerfunc!=NULL){
		trigger->next=proxy->ev_triggerfunc;
	}
	proxy->ev_triggerfunc=trigger;
	return EVENT_OK;
}

void evEnableTrigger(event_base *ev){
	ev->flag=ev->flag | EVENT_TRIGGERABLE;
}

void evDisableTrigger(event_base *ev){
	ev->flag=ev->flag & ~EVENT_TRIGGERABLE;
}

void processTrigger(event_proxy *proxy){
	event_trigger *head=proxy->ev_triggerfunc;
	while(head){
		event_base *ev=head->event;
		if(ev->type==EVENT_TIMEOUT){
			evGenericTimeTriggerProc(proxy,ev,head->clientData);
		}else{
			evGenericUserTriggerProc(proxy,ev,head->clientData);
		}
		head=head->next;
	}
}

int trigger(event_base *ev){
	int fd=*((int*)(&(ev->fd))+1);
	int flags;
	if((flags=fcntl(fd,F_GETFL))==-1)
		return EVENT_ERR;
	flags |= O_NONBLOCK;
	if(fcntl(fd,F_SETFL,flags)==-1)
		return EVENT_ERR;
	char buf[]="1";
	int n;
	if((n=write(fd,buf,1))<=0)
		return EVENT_ERR;
	return EVENT_OK;
}

void eventLoop(event_proxy *proxy){
		if(proxy->maxfd==-1)return;
		struct timeval tval;
		tval.tv_sec=0;
		tval.tv_usec=1000;
		processTrigger(proxy);
		int num=eventPoll(proxy,&tval);
		for(int i=0;i<num;i++){
			event_base *ev=proxy->events[proxy->ev_fires[i].fd];
			if(ev->type==EVENT_FILE){
				int mask=proxy->ev_fires->mask;
				int fd=proxy->ev_fires->fd;
				if(mask & EVENT_READABLE)
				   	((event_file*)ev)->rfileProc(fd,NULL);
				if(mask & EVENT_WRITEABLE)
					((event_file*)ev)->wfileProc(fd,NULL);
				continue;
			}
			if(ev->type==EVENT_TIMEOUT){
				evGenericTimeoutProc(proxy,ev,((event_timeout*)ev)->clientData);
				continue;
			}
			if(ev->type==EVENT_USER){
				evGenericUserProc(proxy,ev,((event_user*)ev)->clientData);
				continue;
			}
			if(ev->type==EVENT_SIGNAL){
				evGenericSignalProc(proxy,ev,((event_signal*)ev)->signo,((event_signal*)ev)->clientData);
				continue;
			}
		}
}
