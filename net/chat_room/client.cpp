#include <libgen.h>//basename
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>//#include <netinet/in.h> //sockaddr_in
#include <unistd.h>//close STDIN_FILENO pipe
#include <poll.h>//pollfd
#include <fcntl.h>
#define BUFFER_SIZE 64
int main(int argc,char* argv[]){
    if(argc <= 2){
        std::cout<<"usage:"<<basename(argv[0])<<" ipaddress portnum"<<std::endl;
        return -1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);
    struct sockaddr_in server_address;
    bzero(&server_address,sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&server_address.sin_addr);
    server_address.sin_port = htons(port);
    int sockfd = socket(PF_INET,SOCK_STREAM,0);
    assert(sockfd>=0);
    if(connect(sockfd,(struct sockaddr*)&server_address,sizeof(server_address)) < 0){
        std::cout<<"connect failed"<<std::endl;
        close(sockfd);
        return 1;
    }
    pollfd fds[2];
    fds[0].fd = STDIN_FILENO;//标准输入
    fds[0].events = POLLIN;
    fds[0].revents=0;
    fds[1].fd = sockfd;
    fds[1].events = POLLIN|POLLHUP;
    fds[1].revents = 0;
    char read_buf[BUFFER_SIZE];
    int pipefd[2];
    int ret = pipe(pipefd);
    assert(ret != -1);
    while (true)
    {
        ret = poll(fds,2,-1);
        if(ret < 0){
            break;
        }
        if(fds[1].revents&POLLHUP){
            std::cout<<"server close the connection"<<std::endl;
            break;
        }
        else if(fds[1].revents&POLLIN){
            memset(read_buf,'\0',BUFFER_SIZE);
            recv(fds[1].fd,read_buf,BUFFER_SIZE-1,0);
            std::cout <<read_buf<<std::endl;
        }
        if(fds[0].revents&POLLIN){
            ret = splice(STDIN_FILENO,nullptr,pipefd[1],nullptr,32768,SPLICE_F_MORE|SPLICE_F_MOVE);
            ret = splice(pipefd[0],nullptr,sockfd,nullptr,32768,SPLICE_F_MORE|SPLICE_F_MOVE);
        }
    }
    close(sockfd);
    return 0;
}