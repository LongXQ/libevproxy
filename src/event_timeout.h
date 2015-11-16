typedef struct event_time_proxy{
	int type;
	int next_id;
	event_time* events;
}event_time_proxy;

typedef struct event_time{
	int type;
	int fd[2]; //fd[0]是要注册到epoll中去的fd，fd[1]是写端，由事件可以触发的时候，往这个写端写数据来触发事件发生，这样epoll就可以通过检测到fd[0]知道事件发生了。
	int falg; //也许可以和type合并成一个字段，具体的解释请参加file_event结构
	int mask;
	int id;
	long long when_sec;
	long long when_ms;
	void *clientData;
}event_time;
