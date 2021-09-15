
#include <libgen.h>//basename
#include <iostream>
#include <cstdlib>
#include <netinet/in.h> //sockaddr_in
#include <cstring>

int main(int argc,char* argv[]){
    if(argc <= 2){
        std::cout<<"usage:"<<basename(argv[0])<<" ipaddress portnum"<<std::endl;
        return -1;
    }

    const char* ip = argv[1];
    int port = atoi(argv[2]);
    struct sockaddr_in server_address;
    bzero(&server_address,sizeof(server_address));
    
    return 0;
}