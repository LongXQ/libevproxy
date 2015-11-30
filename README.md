# libevproxy
libevproxy是一个用C语言编写的，基于reactor模式的事件驱动库，目前只支持Linux平台。支持I/O多路复用技术，不过目前只支持epoll和select。
支持各种不同类型的事件，目前支持的有I/O事件，定时器事件，信号事件和用户自定义事件。
### livevproxy使用
##### 创建一个事件代替
首先我们创建一个事件代替，这个代替就是负责管理各种事件的角色。
```c
#include "event.h"
event_proxy *proxy=createEventProxy(setsize);
```
参数setsize表示这个代理默认管理的事件的个数的大小，当然如果后续添加的事件超过这个大小，代理会自动管理的。而不用重新设置大小。
#####  创建一个事件
第二步就是创建我们需要的事件类型
```c
// 创建一个I/O事件，fd为I/O事件对应的文件描述符
event_base *event=createEventFile(proxy,fd);
```
```c
// 创建一个定时器事件，ms为定时的时间
event_base *event=createEventTimeout(proxy,ms);
```
```c
// 创建一个信号事件，signo为对应的信号，sigset为信号事件触发的时候需要阻塞的信号集
event_base *event=createEventSignal(proxy,signo,sigset);
```
```c
// 创建一个用户自定义事件，maxtimes为最多触发的次数，triggerProc为用户自定义的触发函数，clientData为传给triggerProc的参数
event_base *event=createEventUser(proxy,maxtimes,triggerProc,clientData);
```
##### 注册事件
创建完一个事件后，接着就是注册到代理中了
```c
// 注册I/O事件，mask为需要触发的事件类型，如可读或可写。fileProc为事件发生时执行的处理函数，clientData为传给fileProc的参数
addFileEvent(proxy,event,mask,fileProc,clientData);
```
```c
//注册信号事件，signaProc为信号事件发生时的处理函数
addSignalEvent(proxy,event,signalProc,clientData);
```
```c
// 注册一个定时器事件，timeProc为定时器事件发生时的处理函数
addTimeoutEvent(proxy,event,timeProc,*clientData);
```
```c
// 注册一个用户自定义事件，userProc为事件发生时的处理函数，cleanProc为清理函数
addUserEvent(proxy,ev,userProc,cleanProc,clientData);
```
##### 启动代理
最后一步就是启动代理了，这之后所有的事件就会被代理监控，如果发生了，就会执行注册时的函数
```c
eventLoop(proxy);
```

目前该事件库还在完善中，如果有兴趣的同学可以一起完善。如果有问题可以联系我。