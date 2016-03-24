//
// Created by herbertqiao on 3/21/16.
//

#ifndef UDTWRAPPER_UEPOLL_HH
#define UDTWRAPPER_UEPOLL_HH

#include <udt.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>

class UEpoll
{
public:

    UEpoll();

    ~UEpoll();

    void Loop();

private:
    int epoll_id;
    std::set<UDTSOCKET> udt_sockets;
    std::set<int> tcp_sockets;
};


#endif //UDTWRAPPER_UEPOLL_HH
