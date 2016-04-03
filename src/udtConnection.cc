//
// Created by herbertqiao on 3/21/16.
//

#include "udtConnection.hh"
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <error.hh>

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
    Close();
}

int udtConnection::Bind(uint32_t address, uint16_t port)
{
    //Load Address
    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(port);
    memcpy(&(bind_addr.sin_addr), &address, sizeof(uint32_t));

    //Generate udp socket
    udp_socket = socket(PF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0)
        throw std::runtime_error("Cannot Init UDP Socket.");

    //Set transparent to allow tproxy
    int opt = 1;
    setsockopt(udp_socket, SOL_IP, IP_TRANSPARENT, &opt, sizeof(opt));
    setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //Bind udp_socket
    if (bind(udp_socket, (struct sockaddr *) &bind_addr, sizeof(bind_addr)) < 0)
        throw std::runtime_error("Cannot Bind UDP Socket.");

    //Generate udt socket
    Init();

    //Bind udt to tdp
    if (UDT::bind2(udt_socket, udp_socket) < 0)
        throw std::runtime_error("Cannot Bind UDT to UDP.");
    return 0;
}

int udtConnection::Listen()
{
    if (udt_socket == 0)
        throw std::runtime_error("UDT Listen Error. Not Bind Yet.");
    if (!UDT::listen(udt_socket, 100) == 0)
        throw std::runtime_error("UDT Listen Error.");
    return 0;
}

int udtConnection::Connect(uint32_t address, uint16_t port)
{
    if (udt_socket == 0)
        Init();

    memcpy(&(connect_addr.sin_addr), &address, sizeof(uint32_t));

    connect_addr.sin_family = AF_INET;
    connect_addr.sin_port = htons(port);
    memset(&(connect_addr.sin_zero), '\0', 8);
    if (UDT::ERROR == UDT::connect(udt_socket, (sockaddr *) &(connect_addr), sizeof(connect_addr))) {
        Log::Log(UDT::getlasterror().getErrorMessage(), 5);
        throw UConnect::ConnectionFailed();
    }
    return 0;
}

int udtConnection::Init()
{
    udt_socket = UDT::socket(AF_INET, SOCK_STREAM, 0);
    if (udt_socket == 0)
        throw std::runtime_error("Cannot Init UDT socket.");
    bool blocking = false;
    UDT::setsockopt(udt_socket, 0, UDT_RCVSYN, &blocking, sizeof(bool));
    return udt_socket;
}

int udtConnection::Close()
{
    if (udt_socket != 0) {
        UDT::close(udt_socket);
    }
    if (udp_socket != 0) {
        close(udp_socket);
    }
    udt_socket = 0;
    udp_socket = 0;
    return 0;
}

int udtConnection::Read(char *buffer, uint16_t size)
{
    int s = UDT::recv(udt_socket, buffer, size, 0);
    if (s == UDT::ERROR) {
        auto error = UDT::getlasterror();
        switch (error.getErrorCode()) {
            case 6002:
                throw UConnect::EAgain();
            default:
                throw UConnect::EError();
        }
    }
    string str = "UDT Read ";
    str += std::to_string(s);
    str += " Bits.";
    Log::Log(str, 0);
    return s;
}

int udtConnection::Write(const char *buffer, uint16_t size)
{
    int s = UDT::send(udt_socket, buffer, size, 0);
    if (s == UDT::ERROR) {
        auto error = UDT::getlasterror();
        switch (error.getErrorCode()) {
            case 6002:
                throw UConnect::EAgain();
            default:
                throw UConnect::EError();
        }
    }
    string str = "UDT Write ";
    str += std::to_string(s);
    str += " Bits.";
    Log::Log(str, 0);
    return s;
}