//
// Created by herbertqiao on 3/21/16.
//

#ifndef UDTWRAPPER_UDTCONNECTION_HH
#define UDTWRAPPER_UDTCONNECTION_HH

#include <iostream>
#include <udt.h>
#include "head.hh"

using namespace std;

class udtConnection
{
public:
    udtConnection();

    udtConnection(UDTSOCKET socket);

    ~udtConnection();

    int GetSocket() const
    {
        return udt_socket;
    }

    void SetSocket(int s)
    {
        udt_socket = s;
    }

    void SetStatus(Connection::ConnectionStatus s)
    {
        status = s;
    }

    int GetEvent() const
    {
        return event;
    }

    void SetEvent(int e)
    {
        event = e;
    }

    void SendFin()
    {
        fin = true;
    }

    int Connect(uint32_t address, uint16_t port);

    int Bind(uint32_t address, uint16_t port);

    int Listen();

    int Close();

    int Read(char *buffer, uint16_t size);

    int Write(const char *buffer, uint16_t size);

    friend class Session;

private:

    int Init();

    UDTSOCKET udt_socket;
    UDPSOCKET udp_socket;

    int event = EPOLLOpt::UDT_EPOLL_IN | EPOLLOpt::UDT_EPOLL_ERR;

    Connection::ConnectionStatus status;

    struct sockaddr_in bind_addr;
    struct sockaddr_in connect_addr;

    bool fin = false;
};

#endif //UDTWRAPPER_UDTCONNECTION_HH
