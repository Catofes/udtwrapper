//
// Created by herbertqiao on 3/24/16.
//

#include "tcpConnection.hh"
#include "error.hh"
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>


tcpConnection::tcpConnection()
{
    tcp_socket = 0;
}

tcpConnection::tcpConnection(int socket)
{
    tcp_socket = socket;
}

tcpConnection::~tcpConnection()
{
    Close();
}

int tcpConnection::Bind(uint32_t address, uint16_t port)
{
    //Load Address
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(port);
    memcpy(&(bind_addr.sin_addr), &address, sizeof(uint32_t));

    //Generate udp socket
    Init();

    //Set transparent to allow tproxy
    int opt = 1;
    setsockopt(tcp_socket, SOL_IP, IP_TRANSPARENT, &opt, sizeof(opt));

    //Bind udp_socket
    if (bind(tcp_socket, (struct sockaddr *) &bind_addr, sizeof(bind_addr)) < 0)
        throw std::runtime_error("Cannot Bind TCP Socket.");
    return 0;
}

int tcpConnection::Listen()
{
    if (tcp_socket == 0)
        throw std::runtime_error("TCP Listen Error. Not Bind Yet.");
    if (!listen(tcp_socket, 100) == 0)
        throw std::runtime_error("TCP Listen Error.");
    return 0;
}

int tcpConnection::Connect(uint32_t address, uint16_t port)
{
    if (tcp_socket == 0)
        Init();

    memcpy(&(connect_addr.sin_addr), &address, sizeof(uint32_t));

    connect_addr.sin_family = AF_INET;
    connect_addr.sin_port = htons(port);
    memset(&(connect_addr.sin_zero), '\0', 8);
    if (!(connect(tcp_socket, (sockaddr *) &(connect_addr), sizeof(connect_addr))) == 0) {
        Log::Log(strerror(errno), 5);
        throw TConnect::ConnectionFailed();
    }
    return 0;
}

int tcpConnection::Init()
{
    tcp_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (tcp_socket == 0)
        throw std::runtime_error("Cannot Init TCP socket.");
    return tcp_socket;
}

int tcpConnection::Close()
{
    if (tcp_socket != 0) {
        close(tcp_socket);
    }
    tcp_socket = 0;
    return 0;
}

int tcpConnection::Read(char *buffer, uint16_t size)
{
    int s = (int) recv(tcp_socket, buffer, size, 0);
    if (s < 0) {
        switch (errno) {
            case EAGAIN:
                throw TConnect::EAgain();
            default:
                throw TConnect::EError();
        }
    }
    return s;
}

int tcpConnection::Write(char *buffer, uint16_t size)
{
    int s = (int) send(tcp_socket, buffer, size, 0);
    if (s < 0) {
        switch (errno) {
            case EAGAIN:
                throw TConnect::EAgain();
            default:
                throw TConnect::EError();
        }
    }
    return s;
}
