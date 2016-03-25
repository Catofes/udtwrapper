//
// Created by herbertqiao on 3/25/16.
//

#include <iostream>
#include "epoll.hh"

using namespace std;

int main()
{
    UEpoll epoll;
    epoll.InitServer("0.0.0.0", 9000);
    epoll.SetDestination("127.0.0.1", 9001);
    epoll.Loop();
}