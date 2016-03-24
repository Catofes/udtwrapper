//
// Created by herbertqiao on 3/21/16.
//

#include "udtConnection.hh"
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

udtConnection::udtConnection()
{
    udt_socket = 0;
    udp_socket = 0;
}

udtConnection::udtConnection(UDTSOCKET socket)
{
    udt_socket = socket;
    UDT::getsockstate(socket);
}

udtConnection::~udtConnection()
{
    Shutdown();
}

int udtConnection::Bind(std::string address, uint16_t port)
{
    //Load Address
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(port);
    if (!inet_aton(address.c_str(), &(bind_addr.sin_addr)))
        throw std::runtime_error("Unknow Address.");

    //Generate udp socket
    udp_socket = socket(PF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0)
        throw std::runtime_error("Cannot Init UDP Socket.");

    //Set transparent to allow tproxy
    int opt = 1;
    setsockopt(udp_socket, SOL_IP, IP_TRANSPARENT, &opt, sizeof(opt));

    //Bind udp_socket
    if (bind(udp_socket, (struct sockaddr *) &bind_addr, sizeof(bind_addr)) < 0)
        throw std::runtime_error("Cannot Bind UDP Socket.");

    //Generate udt socket
    udt_socket = UDT::socket(AF_INET, SOCK_STREAM, 0);
    if (udt_socket < 0)
        throw std::runtime_error("Cannot Init UDT Socket.");

    //Bind udt to tdp
    if (UDT::bind2(udt_socket, udp_socket) < 0)
        throw std::runtime_error("Cannot Bind UDT to UDP.");
    return 0;
}

int udtConnection::Listen()
{
    if (udt_socket == 0)
        throw std::runtime_error("Listen Error. Not Bind Yet.");
    UDT::listen(udt_socket, 100);
    return 0;
}

int udtConnection::Connect(std::string address, uint16_t port)
{
    if (udt_socket == 0)
        Init();

    if (inet_aton(address.c_str(), &(connect_addr.sin_addr)) == 0) {
        cout << "[E] Wrong Ip Address." << endl;
        return -1;
    }
    connect_addr.sin_family = AF_INET;
    connect_addr.sin_port = htons(port);
    memset(&(connect_addr.sin_zero), '\0', 8);
    if (UDT::ERROR == connect(udt_socket, (sockaddr *) &(connect_addr), sizeof(connect_addr))) {
        cout << "[E] connection failed." << endl;
        cout << "[D] " << UDT::getlasterror().getErrorMessage() << endl;
        return -1;
    }
    return 0;
}

int udtConnection::Init()
{
    udt_socket = UDT::socket(AF_INET, SOCK_STREAM, 0);
    if (udt_socket == 0)
        throw std::runtime_error("Cannot Init UDT socket.");
    return 0;
}

int udtConnection::Shutdown()
{
    if (udt_socket != 0) {
        UDT::close(udt_socket);
    }
    if (udp_socket != 0) {
        close(udp_socket);
    }
}