#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(){
	int port=6379;
	int number=1024;
	while(number){
		char buf[10]="xxx";
		int fd=socket(AF_INET,SOCK_DGRAM,0);
		struct sockaddr_in sin;
		sin.sin_addr.s_addr=INADDR_ANY;
		sin.sin_port=htons(port++);
		sin.sin_family=AF_INET;
		sendto(fd,buf,strlen(buf),0,(struct sockaddr*)&sin,sizeof(sin));
		//recvfrom(fd,buf,strlen(buf),0,(struct sockaddr*)&sin,sizeof(sin));
		printf("send ok\n");
		number--;
	}
		return 0;
}
