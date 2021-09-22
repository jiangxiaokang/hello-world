#include "net_utility.h"
#include <fcntl.h>
#include <sys/epoll.h>

namespace NET_UTILITY{
     int setnonblocking(int fd){
         int old_option = fcntl(fd,F_GETFL);
         int new_option = old_option|O_NONBLOCK;
         fcntl(fd,F_SETFL,new_option);
         return old_option;
     }

     void addepollfd(int epollfd,int fd){
         epoll_event event;
         event.data.fd = fd;
         event.events = EPOLLIN|EPOLLET:
         epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
         setnonblocking(fd);
     }
}