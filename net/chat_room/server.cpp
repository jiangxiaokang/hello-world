#include <iostream>
#include <libgen.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cstring>
#include <poll.h>

#define USER_LIMIT 5
#define BUFFER_SIZE 64
#define FD_LIMIT 65535/*文件描述符数量限制*/

struct client_data{
    sockaddr_in address;
    char* write_buf;
    char buf[BUFFER_SIZE];
};

int setnonblocking(int fd){
    int old_option = fcntl(fd,F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

int main(int argc,char* argv[]){
    if(argc!=3){
        std::cout<<"usage:%s"<<basename(argv[0])<<" ip_address port_number"<<std::endl;
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port = htons(port);
    int listenfd = socket(PF_INET,SOCK_STREAM,0);
    assert(listenfd>=0);
    int ret = bind(listenfd,(struct sockaddr*)&address,sizeof(address));
    assert(ret!=-1);
    ret = listen(listenfd,USER_LIMIT);
    assert(ret!=-1);
    client_data* users = new client_data[FD_LIMIT];
    pollfd fds[USER_LIMIT+1];
    int user_counter = 0;
    for(int i =0;i<=USER_LIMIT;++i){
        fds[i].fd = -1;
        fds[i].events = 0;
    }
    fds[0].fd = listenfd;
    fds[0].events = POLLIN|POLLERR;
    fds[0].revents = 0;
    
    return 0;
}