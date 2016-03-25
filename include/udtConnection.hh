//
// Created by herbertqiao on 3/21/16.
//

#ifndef UDTWRAPPER_UDTCONNECTION_HH
#define UDTWRAPPER_UDTCONNECTION_HH

#include <iostream>
#include <udt.h>

using namespace std;

class udtConnection
{
public:
    udtConnection();

    udtConnection(UDTSOCKET socket);

    ~udtConnection();

    inline int GetSocket()
    {
        return udt_socket;
    }

    inline void SetSocket(int s)
    {
        udt_socket = s;
    }

    int Connect(std::string address, uint16_t port);

    int Bind(std::string address, uint16_t port);

    int Listen();

    int Close();

    friend class Session;

private:

    int Init();

    UDTSOCKET udt_socket;
    UDPSOCKET udp_socket;

    struct sockaddr_in bind_addr;
    struct sockaddr_in connect_addr;
};

#endif //UDTWRAPPER_UDTCONNECTION_HH
