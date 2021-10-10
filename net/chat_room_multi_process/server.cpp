#include <iostream>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <assert.h>
#include <libgen.h>
#include "../include/net_utility.h"
#include <sys/event.h>

#define USER_LIMIT 5
#define BUFFER_SIZE 1024
#define FD_LIMIT 65535
#define PROCESS_LIMIT 65535
#define MAX_EVENT_NUMBER 1024

struct client_data
{
    sockaddr_in address; /*客户端socket地址*/
    int connfd;          /*socket 文件描述符*/
    pid_t pid;           /*子进程pid*/
    int pipefd[2];       /*和父进程通信的管道*/
};
static const char *shm_name = "/my_shm";
int sig_pipefd[2];
int shmfd;
char *share_mem = 0;
client_data *users = nullptr;
int *sub_process = nullptr;
int user_count = 0;
bool stop_child = false;
int listenfd;
int kq; /*kqueue*/

void sig_handler(int sig)
{
    int save_errno = errno;
    int msg = sig;
    send(sig_pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}

void addsig(int sig, void (*handler)(int), bool restart = true)
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if (restart)
    {
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, nullptr) != -1);
}

void del_resource()
{
    close(sig_pipefd[0]);
    close(sig_pipefd[1]);
    close(listenfd);
    close(kq);
    shm_unlink(shm_name);
    delete[] users;
    delete[] sub_process;
}

void child_term_handler(int sig)
{
    stop_child = true;
}

int run_child(int idx, client_data *users, char *share_mem)
{
    //子进程
    int kq = kqueue();
    int connfd = users[idx].connfd;
    int pipefd = users[idx].pipefd[1];
    NET_UTILITY::AddFdKqueue(kq,{connfd,pipefd});
    addsig(SIGTERM,child_term_handler,false);
    struct kevent event_list[MAX_EVENT_NUMBER];
    while(!stop_child){
        int number = kevent(kq,nullptr,0,event_list,MAX_EVENT_NUMBER,nullptr);
        if(number < 0 && errno != EINTR){
            std::cerr<<"epoll failure"<<std::endl;
            break;
        }
        for(int i = 0; i<number;++i){
            int sockfd = event_list[i].ident;
            if( (sockfd == connfd) && (event_list[i].flags & EVFILT_READ)){
                memset(share_mem + idx*BUFFER_SIZE,'\0',BUFFER_SIZE);
                int ret = recv(connfd,share_mem+idx*BUFFER_SIZE,BUFFER_SIZE-1,0);
                if(ret < 0){
                    if(errno!= EAGAIN){
                        stop_child = true;
                    }
                }
                else if(ret == 0){
                    stop_child = true;
                }
                else{
                    send(pipefd,(char*)&idx,sizeof(idx),0);
                }
            }
            else if((sockfd == pipefd) && (event_list[i].flags&EVFILT_READ)){
                int client = 0;
                int ret = recv(sockfd,(char*)&client,sizeof(client),0);
                if(ret<0){
                    if(errno!= EAGAIN){
                        stop_child = true;
                    }
                }
                else if(ret == 0){
                    stop_child = true;
                }
                else{
                    send(connfd,share_mem+client*BUFFER_SIZE,BUFFER_SIZE,0);
                }
            }
            else{
                continue;
            }
        }
    }
    close(connfd);
    close(pipefd);
    close(kq);
    return 0;
}

void handle_new_connection(int listenfd)
{
    struct sockaddr_in client_adddress;
    socklen_t client_addrlength = sizeof(client_adddress);
    int connfd = accept(listenfd, (struct sockaddr *)&client_adddress, &client_addrlength);
    if (connfd < 0)
    {
        std::cerr << "errno is : " << connfd << std::endl;
        return;
    }
    if (user_count >= USER_LIMIT)
    {
        const char *info = "too many users\n";
        std::cout << info << std::endl;
        send(connfd, info, strlen(info), 0);
        close(connfd);
        return;
    }
    users[user_count].address = client_adddress;
    users[user_count].connfd = connfd;
    int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, users[user_count].pipefd);
    pid_t pid = fork();
    if (pid < 0)
    {
        close(connfd);
        return;
    }

    if (pid == 0)
    {
        /*child process*/
        close(listenfd);
        close(kq);
        close(users[user_count].pipefd[0]);
        close(sig_pipefd[0]);
        close(sig_pipefd[1]);
        run_child(user_count, users, share_mem);
        munmap((void *)share_mem, USER_LIMIT * BUFFER_SIZE);
        exit(0);
    }
    else
    {
        /*parent process*/
        close(connfd);
        close(users[user_count].pipefd[1]);
        NET_UTILITY::AddFdKqueue(kq, {users[user_count].pipefd[0]});
        users[user_count].pid = pid;
        sub_process[pid] = user_count;
        user_count++;
    }
}

void handle_sig_event(int sig_fd,bool& terminate,bool& stop_server)
{
    /*信号事件*/
    char signals[1024];
    int ret = recv(sig_pipefd[0], signals,sizeof(signals),0);
    if(ret <= 0){
        return;
    }
    for(int i =0;i<ret;++i){
        switch (signals[i])
        {
        case SIGCHLD:
        {
            /*子进程退出，客户端连接关闭*/
            pid_t pid;
            int stat;
            while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
            {
                int del_user_index = sub_process[pid];
                sub_process[pid] = -1;
                if (del_user_index < 0 || del_user_index > USER_LIMIT)
                {
                    continue;
                }
                client_data &data = users[del_user_index];
                struct kevent ev_set;
                EV_SET(&ev_set, data.pipefd[0], EVFILT_READ, EV_DELETE, 0, 0, nullptr);
                if(kevent(kq,&ev_set,1,nullptr,0,nullptr)==-1){
                    std::cerr<<"delete event error"<<std::endl;
                }
                close(data.pipefd[0]);
                /*最后一位的连接替换当前断开的连接*/
                users[del_user_index] = users[--user_count];
                sub_process[users[del_user_index].pid] = del_user_index;
            }
            if(terminate && user_count == 0){
                stop_server = true;
            }
        }
            break;
        case SIGTERM:
        case SIGINT:
        {
            /*结束服务器程序*/
            std::cout<<"kill all the child now"<<std::endl;
            if(user_count == 0){
                stop_server = true;
                break;
            }
            for(int i =0;i<user_count;++i){
                int pid = users[i].pid;
                kill(pid,SIGTERM);
            }
            terminate = true;
            break;
        }
        default:
            break;
        }
    }
}

void handle_child_event(int fd){
    int child = 0;
    int ret = recv(fd,(char*)&child,sizeof(child),0);
    std::cout<<"read data from child across pipe id = "<<child<<std::endl;
    if(ret<=0){
        return;
    }
    for(int i =0;i<user_count;++i){
        if(users[i].pipefd[0]== fd){
            continue;
        }
        std::cout<<"send data to child across pipe"<<std::endl;
        send(users[i].pipefd[0],(char*)&child,sizeof(child),0);
    }
}

int main(int argc, char *argv[])
{
    if (argc <= 2)
    {
        std::cout << "usage:" << basename(argv[0]) << " ip_address port_number" << std::endl;
        return 1;
    }
    listenfd = NET_UTILITY::InitSocket(argv[1], argv[2], SOCK_STREAM, 5);
    user_count = 0;
    users = new client_data[USER_LIMIT + 1];
    sub_process = new int[PROCESS_LIMIT];
    for (int i = 0; i < PROCESS_LIMIT; ++i)
    {
        sub_process[i] = -1;
    }

    kq = kqueue();
    if (kq < 0)
    {
        std::cerr << "kqueue error" << std::endl;
        return 1;
    }
    struct kevent event_list[MAX_EVENT_NUMBER];
    int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, sig_pipefd);
    NET_UTILITY::setnonblocking(sig_pipefd[1]);
    NET_UTILITY::AddFdKqueue(kq, {listenfd, sig_pipefd[0]});
    addsig(SIGCHLD, sig_handler);
    addsig(SIGTERM, sig_handler);
    addsig(SIGINT, sig_handler);
    addsig(SIGPIPE, SIG_IGN); /*服务器断开链接，客户端发数据，触发SIGPIPE信号*/
    bool stop_server = false;
    bool terminate = false;
    shmfd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    ret = ftruncate(shmfd, USER_LIMIT * BUFFER_SIZE);
    share_mem = (char *)mmap(NULL, USER_LIMIT * BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    close(shmfd);
    struct kevent ev_set;
    while (!stop_server)
    {
        int nev = kevent(kq, nullptr, 0, event_list, MAX_EVENT_NUMBER, nullptr);
        if (nev < 1)
        {
            std::cout << "kevent error, nev = " << nev << std::endl;
            break;
        }
        for (int i = 0; i < nev; ++i)
        {
            struct kevent &event_info = event_list[i];
            /*new connection*/
            if (event_info.ident == listenfd)
            {
                handle_new_connection(listenfd);
            }
            else if (event_info.ident == sig_pipefd[0] && (event_info.flags & EVFILT_READ))
            {
                /*信号*/
                handle_sig_event(sig_pipefd[0], terminate, stop_server);
            }
            else if (event_info.flags & EVFILT_READ)
            {
                /*子进程向父进程写入数据*/
                handle_child_event(event_info.ident);
            }
        }
    }
    del_resource();
    return 0;
}