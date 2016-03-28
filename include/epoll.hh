//
// Created by herbertqiao on 3/21/16.
//

#ifndef UDTWRAPPER_UEPOLL_HH
#define UDTWRAPPER_UEPOLL_HH

#include <udt.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "tcpConnection.hh"
#include "udtConnection.hh"
#include "sessionManager.hh"

class UEpoll
{

public:

    UEpoll();

    ~UEpoll();

    void InitServer(string listen_address, uint16_t listen_port);

    void InitClient(string listen_address, uint16_t listen_port);

    void SetDestination(string remote_address, uint16_t remote_port);

    void Loop();

private:
    enum EpollType
    {
        Server,
        Client
    };

    void HandleUdtRead();

    void HandleUdtWrite();

    void HandleTcpRead();

    void HandleTcpWrite();

    void AcceptTcp();

    void AcceptUdt();

    int epoll_id;

    EpollType type;

    SessionManager sessionManager;

    set<int> udt_read_fds, udt_write_fds, tcp_read_fds, tcp_write_fds;

    tcpConnection *tcp;
    udtConnection *udt;

};


#endif //UDTWRAPPER_UEPOLL_HH
