#ifndef EVENT_INTERFACE_HEADER
#define EVENT_INTERFACE_HEADER

#define EVENT_FILE 0X01
#define EVENT_TIMEOUT 0x02
#define EVENT_SIGNAL 0x04
#define EVENT_USER 0x08
#define EVENT_OK 1
#define EVENT_ERR 0
typedef struct event_base{
	int type;
	int fd;
	char data[20];
}event_base;

typedef struct event_proxy_base {
	int type;
	char data[16];
}event_proxy_base;

#endif