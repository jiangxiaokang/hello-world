#include "time_wheel.h"
#include <iostream>
time_wheel::time_wheel(){

}

time_wheel::~time_wheel(){
    for(int i =0;i<N;++i){
        tw_timer* tmp = slots[i];
        while(tmp){
            slots[i] = tmp->next;
            delete tmp;
            tmp = slots[i];
        }
    }
}

tw_timer* time_wheel::add_timer(int time_out){
    if(time_out < 0){
        return nullptr;
    }
    int ticks = 0;
    if(time_out < SI){
        ticks = 1;
    }
    else{
        ticks = time_out/SI;
    }
    int rotation = ticks/N;
    int ts = (cur_slot + (ticks%N))%N;
    tw_timer* timer = new tw_timer(rotation,ts);
    if(!slots[ts]){
        std::cout<<"add timer,rotation is "<<rotation<<" ,cur_slot is "<<cur_slot
        <<" ts is "<<ts<<std::endl;
        slots[ts] = timer;
    }
    else{
        timer->next = slots[ts];
        slots[ts]->prev = timer;
        slots[ts] = timer;
    }
    return timer;
}

void time_wheel::del_timer(tw_timer* timer){
    if(!timer){
        return ;
    }
    int ts = timer->time_slot;
    if(timer == slots[ts]){
        slots[ts]=slots[ts]->next;
    }
    if(slots[ts]){
        slots[ts]->prev = nullptr;
    }
    delete timer;
}

void time_wheel::tick(){
    tw_timer* tmp = slots[cur_slot];
    while(tmp){
        if(tmp->rotation > 0){
            tmp->rotation --;
            tmp = tmp->next;
        }
        else{
            tmp->cb_func(tmp->user_data);
            if(tmp == slots[cur_slot]){
                slots[cur_slot] = tmp->next;
                delete tmp;
                if(slots[cur_slot]){
                    slots[cur_slot]->prev = nullptr;
                }
                tmp = slots[cur_slot];
            }
            else{
                tmp->prev->next = tmp->next;
                if(tmp->next){
                    tmp->next->prev = tmp->prev;
                }
                tw_timer* tmp2 = tmp->next;
                delete tmp;
                tmp = tmp2;
            }
        }
    }
    cur_slot = (++cur_slot)%N;
}

int main(){
    return 0;
}