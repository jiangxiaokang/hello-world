#ifndef _NET_UTILITY_H_
#define _NET_UTILITY_H_

namespace NET_UTILITY{
    int setnonblocking(int fd);
    void addepollfd(int epollfd,int fd);
}

#endif