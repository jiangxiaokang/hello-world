#ifndef _NET_UTILITY_H_
#define _NET_UTILITY_H_
#include <vector>
namespace NET_UTILITY{
    int setnonblocking(int fd);
  //  void addepollfd(int epollfd,int fd);
    int InitSocket(const char *ip, const char *port_str, int sock_type, int listen_count);
    bool AddFdKqueue(int kq,const std::vector<int>& fd_vec);
}

#endif