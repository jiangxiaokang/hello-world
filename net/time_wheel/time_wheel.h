#pragma once
#include <arpa/inet.h>
#include <time.h>
#include <functional>

#define BUFFER_SIZE 64
struct client_data{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
};

struct tw_timer{
    tw_timer(int rot,int ts):rotation(rot),time_slot(ts){

    }
    int rotation;
    int time_slot;
    //void (*cd_fnc)(client_data*);
    std::function<void(client_data*)> cb_func;
    client_data* user_data = nullptr;
    tw_timer* next = nullptr;
    tw_timer* prev = nullptr;
};

class time_wheel
{
public:
    time_wheel();
    ~time_wheel();
    tw_timer* add_timer(int timeout);
    void del_timer(tw_timer* timer);
    void tick();
private:
    static const int N = 60;
    static const int SI = 1;
    tw_timer* slots[N] = {nullptr};
    int cur_slot = 0;
};