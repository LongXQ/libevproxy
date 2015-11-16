#ifndef EVENT_INTERFACE_HEADER
#define EVENT_INTERFACE_HEADER

#define EVENT_FILE 0X01
#define EVENT_TIMEOUT 0x02
#define EVENT_SIGNAL 0x04
#define EVENT_USER 0x08

#define EV_READABLE 0x01
#define EV_WRITEABLE 0x02
#define EV_NEEDTRIGGER 0X10;

#define EV_TRIGGER ox10
#define EV_ACTIVE 0x02
#define EV_STOP ox01
#define EV_NONE Ox00

#define EVENT_OK 1
#define EVENT_ERR 0

#define SHRINK_RATIO 0.3
typedef struct event_base{
	int type;
	int id;
	int flag; 
	int mask;
	int fd;
	char data[20];
}event_base;

typedef struct event_proxy_base {
	int type;
	char data[16];
}event_proxy_base;

void _initEvents(event_base *ev,int setsize,int prevsetsize);
#endif