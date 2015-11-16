typedef struct event_sig_proxy{
	int type;
	event_sig *signals;
}event_sig_proxy;

typedef struct event_sig{
	int type;
	int fd[2];
	sigset_t sigset;
	sigset_t pending;
	int mask;
}event_sig;
