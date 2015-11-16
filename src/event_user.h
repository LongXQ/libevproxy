typedef struct event_user_proxy{
	int type;
	event_user *events;
	int next_id; //每一个id用来表示用户注册的事件
}event_user_proxy;

typedef struct event_user{
	int type;
	int fd[2];
	char *user_name;
	int mask;
	int id;
}event_user;