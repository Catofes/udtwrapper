//
// Created by herbertqiao on 3/24/16.
//

#ifndef UDTWRAPPER_TCPCONNECTION_HH
#define UDTWRAPPER_TCPCONNECTION_HH

#include <iostream>
#include <udt.h>
#include "head.hh"

class tcpConnection
{
public:
    tcpConnection();

    tcpConnection(int socket);

    ~tcpConnection();

    inline int GetSocket()
    {
        return tcp_socket;
    }

    inline void SetSocket(int socket)
    {
        tcp_socket = socket;
    }

    inline void SetStatus(Connection::ConnectionStatus s)
    {
        status = s;
    }

    inline int GetEvent()
    {
        return event;
    }

    inline void SetEvent(int e)
    {
        event = e;
    }

    inline void SendFin()
    {
        shutdown(tcp_socket, SHUT_WR);
    }

    inline void SetFin()
    {
        fin = true;
    }

    int Connect(uint32_t address, uint16_t port);

    int Bind(uint32_t address, uint16_t port);

    int Listen();

    int Close();

    int Read(char *buffer, uint16_t size);

    int Write(char *buffer, uint16_t size);

    int CheckWrite();

    friend class Session;

private:

    int Init();

    int tcp_socket;

    Connection::ConnectionStatus status;

    int event = EPOLLOpt::UDT_EPOLL_IN | EPOLLOpt::UDT_EPOLL_ERR;

    struct sockaddr_in bind_addr;
    struct sockaddr_in connect_addr;

    bool fin = false;
};


#endif //UDTWRAPPER_TCPCONNECTION_HH
