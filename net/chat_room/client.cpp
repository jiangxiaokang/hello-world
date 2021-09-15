
#include <libgen.h>//basename
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>//#include <netinet/in.h> //sockaddr_in

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
    
    return 0;
}