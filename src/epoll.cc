//
// Created by herbertqiao on 3/21/16.
//

#include <log.hh>
#include <error.hh>
#include <arpa/inet.h>
#include <signal.h>
#include "epoll.hh"


UEpoll::UEpoll()
        : sessionManager(), udt_read_fds(), udt_write_fds(), tcp_read_fds(), tcp_write_fds()
{
    signal(SIGPIPE, SIG_IGN);
    epoll_id = 0;
}

UEpoll::~UEpoll()
{
    UDT::epoll_release(epoll_id);
}

void UEpoll::InitClient(string s, uint16_t p)
{
    type = EpollType::Client;
    epoll_id = UDT::epoll_create();
    if (epoll_id < 0)
        throw std::runtime_error("Cannot Init Epoll.");
    sessionManager.SetEpoll(epoll_id);
    uint32_t local_address;
    uint16_t local_port;
    inet_pton(AF_INET, s.c_str(), &(local_address));
    local_port = p;
    tcp = new tcpConnection();
    tcp->SetEvent(EPOLLOpt::UDT_EPOLL_IN | EPOLLOpt::UDT_EPOLL_ERR);
    try {
        tcp->Bind(local_address, local_port);
        tcp->Listen();
    }
    catch (UException) {
        throw std::runtime_error("Init Client Error. Exiting.");
    }
    int epoll_opt = tcp->GetEvent();
    if (UDT::epoll_add_ssock(epoll_id, tcp->GetSocket(), &epoll_opt) < 0)
        throw std::runtime_error("Can't add socket to epoll. Exiting.");
}

void UEpoll::InitServer(string s, uint16_t p)
{
    type = EpollType::Server;
    epoll_id = UDT::epoll_create();
    if (epoll_id < 0)
        throw std::runtime_error("Cannot Init Epoll.");
    sessionManager.SetEpoll(epoll_id);
    uint32_t local_address;
    uint16_t local_port;
    inet_pton(AF_INET, s.c_str(), &(local_address));
    local_port = p;
    udt = new udtConnection();
    udt->SetEvent(EPOLLOpt::UDT_EPOLL_IN | EPOLLOpt::UDT_EPOLL_ERR);
    try {
        udt->Bind(local_address, local_port);
        udt->Listen();
    }
    catch (UException) {
        throw std::runtime_error("Init Client Error. Exiting.");
    }
    int epoll_opt = udt->GetEvent();
    if (UDT::epoll_add_usock(epoll_id, udt->GetSocket(), &epoll_opt) < 0)
        throw std::runtime_error("Can't add socket to epoll. Exiting.");
}

void UEpoll::SetDestination(string remote_address, uint16_t remote_port)
{
    inet_pton(AF_INET, remote_address.c_str(), &(sessionManager.remote_address));
    sessionManager.remote_port = remote_port;
}

void UEpoll::Loop()
{
    while (true) {
        int count = UDT::epoll_wait(epoll_id, &udt_read_fds, &udt_write_fds, -1, &tcp_read_fds, &tcp_write_fds);
        HandleTcpRead();
        HandleTcpWrite();
        HandleUdtRead();
        HandleUdtWrite();
        sessionManager.GarbageCollection();
        if (count < 0)
            throw std::runtime_error("EPOLL ERROR.");
    }
}

void UEpoll::HandleTcpRead()
{
    for (auto t:tcp_read_fds) {
        //Handle Accept Connection
        if (type == EpollType::Client) {
            //New Connection
            if (t == tcp->GetSocket()) {
                AcceptTcp();
                continue;
            }
        }
        Session *session;
        try {
            session = sessionManager.GetSessionByTcp(t);
        }
        catch (UException) {
            Log::Log("Unknown Session.", 3);
            UDT::epoll_remove_ssock(epoll_id, t);
        }
        try {
            session->HandleTRead();
        }
        catch (UException) {
            session->Rst();
        }
    }
}

void UEpoll::HandleTcpWrite()
{
    for (auto t:tcp_write_fds) {
        Session *session;
        try {
            session = sessionManager.GetSessionByTcp(t);
        }
        catch (UException) {
            Log::Log("Unknown Session.", 3);
            UDT::epoll_remove_ssock(epoll_id, t);
        }
        try {
            session->HandleTWrite();
        }
        catch (UException) {
            session->Rst();
        }
    }
}

void UEpoll::HandleUdtRead()
{
    for (auto u:udt_read_fds) {
        if (type == EpollType::Server) {
            //New Connection
            if (u == udt->GetSocket()) {
                AcceptUdt();
                continue;
            }
        }
        Session *session;
        try {
            session = sessionManager.GetSessionByUdt(u);
        }
        catch (UException) {
            Log::Log("Unknown Session.", 3);
            UDT::epoll_remove_usock(epoll_id, u);
        }
        try {
            session->HandleURead();
        }
        catch (UException) {
            session->Rst();
        }
    }
}

void UEpoll::HandleUdtWrite()
{
    for (auto u:udt_write_fds) {
        Session *session;
        try {
            session = sessionManager.GetSessionByUdt(u);
        }
        catch (UException) {
            Log::Log("Unknown Session.", 3);
            UDT::epoll_remove_usock(epoll_id, u);
        }
        try {
            session->HandleUWrite();
        }
        catch (UException) {
            session->Rst();
        }
    }
}

void UEpoll::AcceptTcp()
{
    struct sockaddr_in client_address;
    socklen_t client_address_length = sizeof(struct sockaddr_in);
    int new_socket = accept(tcp->GetSocket(), (sockaddr *) &client_address, &client_address_length);
    if (new_socket < 0) {
        throw EpollSpace::TcpAcceptError();
    }
    string str = "Accept a connection from :";
    str += string(inet_ntoa(client_address.sin_addr));
    str += ": ";
    str += to_string(ntohs(client_address.sin_port));
    Log::Log(str, 2);

    sessionManager.CreateSessionByTcp(new_socket);
    int epoll_opt = EPOLLOpt::UDT_EPOLL_IN | EPOLLOpt::UDT_EPOLL_ERR;
    if (UDT::epoll_add_ssock(epoll_id, new_socket, &epoll_opt) < 0)
        throw std::runtime_error("Can't add socket to epoll. Exiting.");

}

void UEpoll::AcceptUdt()
{
    struct sockaddr_in client_address;
    int client_address_length = sizeof(struct sockaddr_in);
    int new_socket = UDT::accept(udt->GetSocket(), (sockaddr *) &client_address, &client_address_length);
    if (new_socket < 0) {
        throw EpollSpace::UdtAcceptError();
    }
    string str = "Accept a connection from :";
    str += string(inet_ntoa(client_address.sin_addr));
    str += ": ";
    str += to_string(ntohs(client_address.sin_port));
    Log::Log(str, 2);

    sessionManager.CreateSessionByUdt(new_socket);
    int epoll_opt = EPOLLOpt::UDT_EPOLL_IN | EPOLLOpt::UDT_EPOLL_ERR;
    if (UDT::epoll_add_usock(epoll_id, new_socket, &epoll_opt) < 0)
        throw std::runtime_error("Can't add socket to epoll. Exiting.");
}