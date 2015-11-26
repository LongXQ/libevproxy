#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

#include "evnet.h"
#include "event_interface.h"

int netSetTcpKeepAlive(int fd,int idle,int int interval,int cnt){
	int val=1;
	if(setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,&val,sizeof(val))==-1)
		return NET_ERR;
	val=idle;
	if(setsockopt(fd,IPPROTO_TCP,TCP_KEEPIDLE,&val,sizeof(val))==-1)
		return NET_ERR;
	val=interval;
	if(setsockopt(fd,IPPROTO_TCP,TCP_KEEPINTVL,&val,sizeof(val))==-1)
		return NET_ERR;
	val=cnt;
	if(setsockopt(fd,IPPROTO_TCP,TCP_KEEPCNT,&val,sizeof(val))==-1)
		return NET_ERR;
	return NET_OK;
}

static int netSetTcpNoDelay(int fd,int val){
	if(val<=0)
		val=0;
	else
		val=1;
	if(setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,&val,sizeof(val))==-1)
		return NET_ERR;
	return NET_OK;
}

int netEnableTcpNoDelay(int fd){
	return netSetTcpNoDelay(fd,1);
}

int netDisableTcpNoDelay(int fd){
	return netSetTcpNoDelay(fd,0);
}

int netSetSendBuffer(int fd,int buffersize){
	if(setsockopt(fd,SOL_SOCKET,SO_SNDBUF,&buffersize,sizeof(buffersize))==-1)
		return NET_ERR;
	return NET_OK;
}

int netSetSendTimeout(int fd,long long ms){
	struct timeval tv;
	tv.tv_sec=ms/1000;
	tv.tv_usec=(ms%1000)*1000;
	if(setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv))==-1)
		return NET_ERR;
	return NET_OK;
}

int netEnableReuseAddr(int fd){
	int yes=1;
	if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes))==-1)
		return NET_ERR;
	return NET_OK;
}

int netSetNonBlock(int fd,int non_block){
	int flags;
	if((flags=fcntl(fd,F_GETFL))==-1)
		return NET_ERR;
	if(non_block)
		flags|=O_NONBLOCK;
	else
		flags&=~O_NONBLOCK;
	if(fcntl(fd,F_SETFL,flags)==-1)
		return NET_ERR;
	return NET_OK;
}

int netTcpNonBlockConnect(char *addr,char *port,char *source_addr,char *source_port,int non_block){
	int rv,s=NET_ERR,bound=0;
	if(!addr || !port)
		return NET_ERR;
	struct addrinfo hints,*servinfo,*clientinfo,*p,*c;
	memset(&hints,0,sizeof(hints));

	hints.ai_family=AF_UNSPCEC //IPV4和IPV6都可以
	hints.ai_socktype=SOCK_STREAM;

	if((rv=getaddrinfo(addr,port,&hints,&servinfo))!=0)
		return NET_ERR;
	for(p=servinfo;p!=NULL;p=p->ai_next){
		if((s=socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1)
			continue;
		if(netEnableReuseAddr(s)==NET_ERR)goto error;
		if(non_block){
			if(netSetNonBlock(s,1)==NET_ERR)
				goto error;
		}
		if(source_addr==NULL)
			hints->ai_flag=AI_PASSIVE;
		if((rv=getaddrinfo(source_addr,source_port,&hints,&clientinfo))!=0)
			goto error;
		for(c=clientinfo;c!=NULL;c=c->ai_next){
			if(bind(s,c->ai_addr,b->ai_addrlen)!=-1){
				bound=1;
				break;
			}
		}
		freeaddrinfo(clientinfo);
		if(!bound)
			goto error;
		if(connect(s,p->ai_addr,p->ai_addrlen)==-1){
			if(errno==EINPROGRESS && non_block)
				break;
			close(s);
			hints->ai_flag=0;
			continue;
		}
		break;
	}
	if(p==NULL)
		goto error;
	freeaddrinfo(servinfo);
	return s;
error:
    freeaddrinfo(servinfo);
    if(s!=-1)
    	close(s);
    s=NET_ERR;
    return s;
}

int netCreateTcpServer(int af,char *bindaddr,char *port,int backlog){
	struct addrinfo hints,*servinfo,*p;
	int s,rv,yes=1;
	memset(&hints,0,sizeof(hints));
	hints.ai_family=af;
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_flags=AI_PASSIVE;
	if((rv=getaddrinfo(bindaddr,port,&hints,&servinfo))==-1)
		return NET_ERR;
	for(p=servinfo;p!=NULL;p=p->ai_next){
		if((s=socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1)
			continue;
		if(af==AF_INET6){
			if(setsockopt(s,IPPROTO_IPV6,IPV6_V6ONLY,&yes,sizeof(yes))==-1){
				close(s);
				goto error;
			}
		}
		if(netEnableReuseAddr(s,1)==NET_ERR)
			goto error;
		if(bind(s,p->ai_addr,p->ai_addrlen)==-1){
			close(s);
			goto error;
		}
		if(listen(s,backlog)==-1){
			close(s);
			goto error;
		}
		goto end;
	}
	if(p==NULL)
		goto error;
	error:
	s=NET_ERR;
	end:
	freeaddrinfo(servinfo);
	return s;
}

static int netGenericAccept(char *err, int s, struct sockaddr *sa, socklen_t *len) {
    int fd;
    while(1) {
        fd = accept(s,sa,len);
        if (fd == -1) {
            if (errno == EINTR)
                continue;
            else {
                return NET_ERR;
            }
        }
        break;
    }
    return fd;
}

int netTcpAccept(char *err, int s, char *ip, size_t ip_len, int *port) {
    int fd;
    struct sockaddr_storage sa;
    socklen_t salen = sizeof(sa);
    if ((fd = anetGenericAccept(err,s,(struct sockaddr*)&sa,&salen)) == -1)
        return ANET_ERR;

    if (sa.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&sa;
        if (ip) inet_ntop(AF_INET,(void*)&(s->sin_addr),ip,ip_len);
        if (port) *port = ntohs(s->sin_port);
    } else {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&sa;
        if (ip) inet_ntop(AF_INET6,(void*)&(s->sin6_addr),ip,ip_len);
        if (port) *port = ntohs(s->sin6_port);
    }
    return fd;
}

int netCreateUnixSockpair(int type,int sockfd[2]){
	if(socketpair(AF_LOCAL,type,0,sockfd)==-1)
		return NET_ERR;
	return NET_OK;
}

int netCreateTcpSocket(int type){
	if(type!=AF_INET || type!=AF_INT6)
		return NET_ERR;
	int s;
	if((s=socket(type,SOCK_STREAM,0))==-1)
		return NET_ERR;
	if(netEnableReuseAddr(s,1)==NET_ERR){
		close(s);
		return NET_ERR;
	}
	return s;
}


int netCreateUdpSocket(int type){
	if(type!=AF_INET || type!=AF_INT6)
		return NET_ERR;
	int s;
	if((s=socket(type,SOCK_DGRAM,0))==-1)
		return NET_ERR;
	return s;
}

int netCreateUnixSocket(char *path){
	int s;
	struct sockaddr_un sa;
	if((s=socket(AF_LOCAL,SOCK_STREAM,0))==-1)
		return NET_ERR;
	memset(&sa,0,sizeof(sa));
	sa.sun_family=AF_LOCAL;
	strncpy(sa.sun_path,path,sizeof(sa.sun_path)-1);
	if(bind(s,(struct sockaddr*)&sa,sizeof(sa))==-1){
		close(s);
		return NET_ERR;
	}
	return s;
}

int netCreateUnixServer(char *path,int backlog,mode_t perm){
	int s;
	if((s=netCreateUnixSocket(path))==NET_ERR)
		return NET_ERR;
	if(listen(s,backlog)==-1){
		close(s);
		return NET_ERR;
	}
	if(perm)
		chmod(sa.sun_path,perm);
	return s;
}
