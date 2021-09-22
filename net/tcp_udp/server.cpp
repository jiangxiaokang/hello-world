#include <iostream>
#include <libgen.h>
#include <arpa/inet.h>
#include "../include/net_utility.h"

#define MAX_EVENT_NUMBER 1024
#define TCP_BUFFER_SIZE 512
#define UDP_BUFFER_SIZE 1024

int InitSocket(const char* ip,const char* port_str,int sock_type,int listen_count){
    int port = atoi(port_str);
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr); 
    address.sin_port = htons(port);
    int listen_fd = socket(PF_INET,sock_type,0);
    int ret = bind(listen_fd,(struct sockaddr*)&address,sizeof(address));
    assert(ret!=-1);
    ret = listen(listen_fd,listen_count);
    assert(ret!=-1);
    return listen_fd;
}

int accept_tcp_conn(int tcp_fd){
    struct sockaddr_in client_address;
    socklen_t client_addr_len = sizeof(client_address);
    int connfd = accept(tcp_fd,(struct sockaddr*)&client_address,&client_addr_len);
    return connfd;
}

void deal_tcp_conn(int sockfd){
    char buf[TCP_BUFFER_SIZE];
    while (true)
    {
        memset(buf, '\0', TCP_BUFFER_SIZE);
        int ret = recv(sockfd, buf, TCP_BUFFER_SIZE - 1, 0);
        if (ret < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            close(sockfd);
            break;
        }
        else if (ret == 0)
        {
            close(sockfd);
        }
        else
        {
            send(sockfd, buf, ret, 0);
        }
    }
}

void deal_udp_conn(int udp_fd){
    char buf[UDP_BUFFER_SIZE];
    memset(buf,'\0',UDP_BUFFER_SIZE);
    struct sockaddr_in client_address;
    socklen_t client_addr_len = sizeof(client_address);
    int ret = recvfrom(udp_fd,buf,UDP_BUFFER_SIZE-1,0,(struct sockaddr*)*client_address,&client_addr_len);
    if(ret > 0){
        sendto(udp_fd,buf,UDP_BUFFER_SIZE-1,0,(struct sockaddr*)&client_address,&client_addr_len);
    }
    else{
        std::cerr<<"recv udp error ret = "<<ret <<std::endl;
    }
}

int main(int argc,char* argv[]){
    if(argc <= 2){
        std::cout<<"usage:"<<basename(argv[0])<<" ip_address port_number"<<std::endl;
        return 1;
    }
   int tcp_fd = InitSocket(argv[1],argv[2],SOCK_STREAM,5);
   int upd_fd = InitSocket(argv[1],argv[2],SOCK_DGRAM,0);
   epoll_event events[MAX_EVENT_NUMBER];
   int epollfd = epoll_create(5);
   assert(epollfd!=-1);
   NET_UTILITY::addepollfd(epollfd,tcp_fd);
   NET_UTILITY::addepollfd(epollfd,upd_fd);
   while(true){
       int number = epoll_wait(epollfd,events,MAX_EVENT_NUMBER,-1);
       if(number < 0){
           std::cerr<<"epoll failuer"<<std::endl;
           break;
       }
       for(int i =0;i<number;++i){
           int sockfd = events[i].data.fd;
           switch (sockfd){
            case tcp_fd:
                {
                    int connfd = accept_tcp_conn(tcp_fd);
                    addfd(epollfd,connfd);
                }
                break;
            case upd_fd:
                deal_udp_conn(upd_fd);
                break;
            default:
                {
                    if(events[i].events & EPOLLIN){
                        deal_tcp_conn(sockfd);
                    }
                    else{
                        std::cerr<<"invalid hanlder"<<std::endl;
                    }
                }
                break;
            }
       }
   }
   close(tcp_fd);
   return 0;
}