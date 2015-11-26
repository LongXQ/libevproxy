#define NET_ERR -1;
#define NET_OK 0;

int netSetTcpKeepAlive(int fd,int idle,int int interval,int cnt);
int netEnableTcpNoDelay(int fd);
int netDisableTcpNoDelay(int fd);
int netSetSendBuffer(int fd,int buffersize);
int netSetSendTimeout(int fd,long long ms);
int netEnableReuseAddr(int fd);
int netSetNonBlock(int fd,int non_block);
int netTcpNonBlockConnect(char *addr,char *port,char *source_addr,char *source_port,int non_block);
int netCreateTcpServer(int af,char *bindaddr,char *port,int backlog);
int netTcpAccept(char *err, int s, char *ip, size_t ip_len, int *port);
int netCreateUnixSockpair(int type,int sockfd[2]);
int netCreateTcpSocket(int type);
int netCreateUdpSocket(int type);
int netCreateUnixSocket(char *path);
int netCreateUnixServer(char *path,int backlog,mode_t perm);
