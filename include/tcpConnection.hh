//
// Created by herbertqiao on 3/24/16.
//

#ifndef UDTWRAPPER_TCPCONNECTION_HH
#define UDTWRAPPER_TCPCONNECTION_HH

#include <iostream>
#include <udt.h>

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

    int Connect(std::string address, uint16_t port);

    int Bind(std::string address, uint16_t port);

    int Listen();

    int Close();

private:

    int Init();

    int tcp_socket;

    struct sockaddr_in bind_addr;
    struct sockaddr_in connect_addr;
};


#endif //UDTWRAPPER_TCPCONNECTION_HH
