#include "net_utility.h"
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <assert.h>
//#include <sys/epoll.h>
#include <sys/event.h>
#include <array>
namespace NET_UTILITY{


     int setnonblocking(int fd){
         int old_option = fcntl(fd,F_GETFL);
         int new_option = old_option|O_NONBLOCK;
         fcntl(fd,F_SETFL,new_option);
         return old_option;
     }

    //  void addepollfd(int epollfd,int fd){
    //      epoll_event event;
    //      event.data.fd = fd;
    //      event.events = EPOLLIN|EPOLLET;
    //      epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    //      setnonblocking(fd);
    //  }

     bool AddFdKqueue(int kq,const std::vector<int>& fd_vec)
     {
         int fd_len = fd_vec.size();
         const int max_array_len  = 100;
         if(fd_len >= max_array_len)
         {
             return false;
         }
         std::array<struct kevent,max_array_len> ev_list;
         for(size_t i =0;i<fd_len;++i)
         {
            struct kevent ev_set;
            EV_SET(&ev_set, fd_vec[i], EVFILT_READ, EV_ADD, 0, 0, nullptr);
            ev_list[i] = ev_set;
            setnonblocking(fd_vec[i]);
         }
         
         if (kevent(kq, ev_list.data(), fd_len, nullptr, 0, nullptr) == -1)
         {
             return false;
         }
         return true;
     }

     int InitSocket(const char *ip, const char *port_str, int sock_type, int listen_count)
     {
         int port = atoi(port_str);
         struct sockaddr_in address;
         bzero(&address, sizeof(address));
         address.sin_family = AF_INET;
         inet_pton(AF_INET, ip, &address.sin_addr);
         address.sin_port = htons(port);
         int listen_fd = socket(PF_INET, sock_type, 0);
         int ret = bind(listen_fd, (struct sockaddr *)&address, sizeof(address));
         assert(ret != -1);
         ret = listen(listen_fd, listen_count);
         assert(ret != -1);
         return listen_fd;
     }
}