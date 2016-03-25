//
// Created by herbertqiao on 3/25/16.
//

#include <iostream>
#include "epoll.hh"

using namespace std;

int main()
{
    UEpoll epoll;
    epoll.InitClient("0.0.0.0", 8999);
    epoll.SetDestination("127.0.0.1", 9000);
    epoll.Loop();
}