//
// Created by herbertqiao on 3/24/16.
//

#include <error.hh>
#include <string.h>
#include "session.hh"
#include "sessionManager.hh"
#include "head.hh"

Session::Session()
{
    Fresh();
}

Session::~Session()
{

}

void Session::Fresh()
{
    active_time = (int) time(NULL);
}

void Session::Close()
{
    Log("Close.", 2);
    status = SessionSpace::CLOSE;
    if (tcp.GetSocket() > 0) {
        UDT::epoll_remove_ssock(manager->GetEpoll(), tcp.GetSocket());
        tcp.Close();
    }
    if (udt.GetSocket() > 0) {
        UDT::epoll_remove_usock(manager->GetEpoll(), udt.GetSocket());
        udt.Close();
    }
    active_time = 0;
}

void Session::Rst()
{
    Log("Rst.", 5);
    Close();
}

void Session::BlockTcp(bool f)
{
    if (f) {
        Log("Enter Tcp Block.", 1);
        tcp.status = Connection::BLOCKING;
        tcp.event = tcp.event | EPOLLOpt::UDT_EPOLL_OUT;
        UDT::epoll_remove_ssock(manager->GetEpoll(), tcp.tcp_socket);
        UDT::epoll_add_ssock(manager->GetEpoll(), tcp.tcp_socket, &(tcp.event));

        udt.event = udt.event & (EPOLLOpt::UDT_EPOLL_OUT | EPOLLOpt::UDT_EPOLL_ERR);
        UDT::epoll_remove_usock(manager->GetEpoll(), udt.udt_socket);
        if (udt.event != 0)
            UDT::epoll_add_usock(manager->GetEpoll(), udt.udt_socket, &(udt.event));
    } else {
        Log("Leave Tcp Block.", 1);
        tcp.status = Connection::PIPE;
        tcp.event = tcp.event & (EPOLLOpt::UDT_EPOLL_IN | EPOLLOpt::UDT_EPOLL_ERR);
        UDT::epoll_remove_ssock(manager->GetEpoll(), tcp.tcp_socket);
        if (tcp.event != 0)
            UDT::epoll_add_ssock(manager->GetEpoll(), tcp.tcp_socket, &(tcp.event));
        if (tcp.fin)
            tcp.SendFin();

        udt.event = udt.event | EPOLLOpt::UDT_EPOLL_IN;
        UDT::epoll_remove_usock(manager->GetEpoll(), udt.udt_socket);
        UDT::epoll_add_usock(manager->GetEpoll(), udt.udt_socket, &(udt.event));
    }
}

void Session::BlockUdt(bool f)
{
    if (f) {
        Log("Enter Udt Block.", 1);
        udt.status = Connection::BLOCKING;
        udt.event = udt.event | EPOLLOpt::UDT_EPOLL_OUT;
        UDT::epoll_remove_usock(manager->GetEpoll(), udt.udt_socket);
        UDT::epoll_add_usock(manager->GetEpoll(), udt.udt_socket, &(udt.event));

        tcp.event = tcp.event & (EPOLLOpt::UDT_EPOLL_OUT | EPOLLOpt::UDT_EPOLL_ERR);
        UDT::epoll_remove_ssock(manager->GetEpoll(), tcp.tcp_socket);
        if (tcp.event != 0)
            UDT::epoll_add_ssock(manager->GetEpoll(), tcp.tcp_socket, &(tcp.event));
    } else {
        Log("Leave Udt Block.", 1);
        udt.status = Connection::PIPE;
        udt.event = udt.event & (EPOLLOpt::UDT_EPOLL_IN | EPOLLOpt::UDT_EPOLL_ERR);
        UDT::epoll_remove_usock(manager->GetEpoll(), udt.udt_socket);
        if (udt.event != 0)
            UDT::epoll_add_usock(manager->GetEpoll(), udt.udt_socket, &(udt.event));

        tcp.event = tcp.event | EPOLLOpt::UDT_EPOLL_IN;
        UDT::epoll_remove_ssock(manager->GetEpoll(), tcp.tcp_socket);
        UDT::epoll_add_ssock(manager->GetEpoll(), tcp.tcp_socket, &(tcp.event));
    }
}


void Session::UploadWrite()
{
    Log("Upload Write.", 1);
    char *data = upload_write_buffer + upload_write_offset;
    int s = 0;
    try {
        switch (direction) {
            case SessionSpace::TCP2UDT:
                s = udt.Write(data, upload_write_length);
                upload_write_offset += s;
                upload_write_length -= s;
                if (upload_write_length == 0)
                    upload_write_offset = 0;
                BlockUdt(false);
                break;
            case SessionSpace::UDT2TCP:
                s = tcp.Write(data, upload_write_length);
                upload_write_offset += s;
                upload_write_length -= s;
                if (upload_write_length == 0)
                    upload_write_offset = 0;
                BlockTcp(false);
                break;
        }
    }
    catch (TConnect::EAgain) {
        switch (direction) {
            case SessionSpace::TCP2UDT:
                BlockUdt(true);
                break;
            case SessionSpace::UDT2TCP:
                BlockTcp(true);
                break;
        }
    }
    catch (TConnect::EError) { Rst(); }
}

void Session::DownloadWrite()
{
    Log("Download Write.", 1);
    char *data = download_write_buffer + download_write_offset;
    int s = 0;
    try {
        switch (direction) {
            case SessionSpace::UDT2TCP:
                s = udt.Write(data, download_write_length);
                download_write_offset += s;
                download_write_length -= s;
                if (download_write_length == 0)
                    download_write_offset = 0;
                BlockUdt(false);
                break;
            case SessionSpace::TCP2UDT:
                s = tcp.Write(data, download_write_length);
                download_write_offset += s;
                download_write_length -= s;
                if (download_write_length == 0)
                    download_write_offset = 0;
                BlockTcp(false);
                break;
        }
    }
    catch (TConnect::EAgain) {
        switch (direction) {
            case SessionSpace::UDT2TCP:
                BlockUdt(true);
                break;
            case SessionSpace::TCP2UDT:
                BlockTcp(true);
                break;
        }
    }
    catch (TConnect::EError) { Rst(); }
}

void Session::UploadRead()
{
    Log("Upload Read.", 1);
    Head head;
    switch (direction) {
        case SessionSpace::TCP2UDT:
            upload_read_offset += tcp.Read(upload_read_buffer + upload_read_offset, DS - upload_read_offset);
            break;
        case SessionSpace::UDT2TCP:
            upload_read_offset += udt.Read(upload_read_buffer + upload_read_offset, DS - upload_read_offset);
            break;
    }
}

void Session::DownloadRead()
{
    Log("Download Read.", 1);
    Head head;
    switch (direction) {
        case SessionSpace::UDT2TCP:
            upload_read_offset += tcp.Read(upload_read_buffer + upload_read_offset, DS - upload_read_offset);
            break;
        case SessionSpace::TCP2UDT:
            upload_read_offset += udt.Read(upload_read_buffer + upload_read_offset, DS - upload_read_offset);
            break;
    }
}

void Session::SendFin(SessionSpace::Direction d)
{
    Head head;
    switch (d) {
        case SessionSpace::TCP2UDT:
            Log("Send Fin to UDT.", 1);
            head.type = HeadSpace::FIN;
            upload_write_length += head.Pack(upload_write_buffer + upload_write_offset + upload_write_length);
            UploadWrite();
            udt.SendFin();
            break;
        case SessionSpace::UDT2TCP:
            Log("Send Fin to TCP.", 1);
            if (tcp.status == Connection::PIPE)
                tcp.SendFin();
            else {
                tcp.SetFin();
            }
            break;
    }
}

void Session::TcpReadInit()
{
    if (direction == SessionSpace::UDT2TCP)
        throw SessionSpace::ErrorStatus();
    udt.Connect(remote_address, remote_port);
    BlockUdt(true);
    status = SessionSpace::CONNECTING;
}

void Session::UdtReadInit()
{
    if (direction == SessionSpace::UDT2TCP)
        throw SessionSpace::ErrorStatus();
    tcp.Connect(remote_address, remote_port);
    BlockTcp(true);
    status = SessionSpace::CONNECTING;
}


void Session::Tcp2Udt()
{
    Head head;
    switch (direction) {
        case SessionSpace::TCP2UDT:
            if (udt.fin)
                throw SessionSpace::SendData2Fin();
            switch (status) {
                case SessionSpace::INIT:
                    break;
                case SessionSpace::CONNECTING:
                    Log("Send Connect Package.", 2);
                    head.type = HeadSpace::CONNECT;
                    if (tproxy) {
                        head.address = remote_address;
                        head.port = remote_port;
                    }
                    upload_write_length += head.Pack(upload_write_buffer + upload_write_offset + upload_write_length);
                    UploadWrite();
                    break;
                case SessionSpace::PIPE:
                    UploadRead();
                    head.type = HeadSpace::DATA;
                    head.data_length = upload_read_offset;
                    upload_write_length += head.Pack(upload_write_buffer + upload_write_offset + upload_write_length);
                    memcpy(upload_write_buffer + upload_write_offset + upload_write_length,
                           upload_read_buffer, upload_read_offset);
                    upload_write_length += upload_read_offset;
                    upload_read_offset = 0;
                    UploadWrite();
                    break;
                default:
                    break;
            }
            break;
        case SessionSpace::UDT2TCP:
            if (udt.fin)
                throw SessionSpace::SendData2Fin();
            DownloadRead();
            head.type = HeadSpace::DATA;
            head.data_length = download_read_offset;
            download_write_length += head.Pack(download_write_buffer + download_write_offset + download_write_length);
            memcpy(download_write_buffer + download_write_offset + download_write_length,
                   download_read_buffer, download_read_offset);
            download_write_length += download_read_offset;
            download_read_offset = 0;
            DownloadWrite();
    }
}

void Session::Udt2Tcp()
{
    Head head;
    switch (direction) {
        case SessionSpace::TCP2UDT:
            while (true) {
                DownloadRead();
                head.Read(download_read_buffer, download_read_offset);
                switch (head.type) {
                    case HeadSpace::DATA:
                        if (status != SessionSpace::PIPE) {
                            throw SessionSpace::ErrorStatus();
                        }
                        if (head.data_length + 3 > download_read_offset)
                            throw HeadSpace::Partial();
                        memcpy(download_write_buffer + download_write_offset + download_write_length,
                               download_read_buffer + 3, head.data_length);
                        if (head.data_length + 3 < download_read_offset) {
                            memcpy(buffer, download_read_buffer + head.data_length + 3,
                                   download_read_offset - (head.data_length + 3));
                            memcpy(download_read_buffer, buffer, download_read_offset - (head.data_length + 3));
                            download_read_offset -= (head.data_length + 3);
                        }
                        UploadWrite();
                        break;
                    case HeadSpace::CONNECT:
                        throw HeadSpace::UnknownType();
                    case HeadSpace::FIN:
                        SendFin(SessionSpace::UDT2TCP);
                        break;
                    case HeadSpace::RST:
                        Close();
                }
            }
        case SessionSpace::UDT2TCP:
            while (true) {
                UploadRead();
                head.Read(upload_read_buffer, upload_read_offset);
                switch (head.type) {
                    case HeadSpace::DATA:
                        if (status != SessionSpace::PIPE) {
                            throw SessionSpace::ErrorStatus();
                        }
                        if (head.data_length + 3 > upload_read_offset)
                            throw HeadSpace::Partial();
                        memcpy(upload_write_buffer + upload_write_offset + upload_write_length,
                               upload_read_buffer + 3, head.data_length);
                        if (head.data_length + 3 < upload_read_offset) {
                            memcpy(buffer, upload_read_buffer + head.data_length + 3,
                                   upload_read_offset - (head.data_length + 3));
                            memcpy(upload_read_buffer, buffer, upload_read_offset - (head.data_length + 3));
                            upload_read_offset -= (head.data_length + 3);
                        }
                        UploadWrite();
                        break;
                    case HeadSpace::CONNECT:
                        if (tproxy) {
                            remote_address = head.address;
                            remote_port = head.port;
                            if (3 < upload_read_offset) {
                                memcpy(buffer, upload_read_buffer + 3, upload_read_offset - 3);
                                memcpy(upload_read_buffer, buffer, upload_read_offset - 3);
                                upload_read_offset -= 3;
                            }
                        }
                        UdtReadInit();
                        break;
                    case HeadSpace::FIN:
                        SendFin(SessionSpace::UDT2TCP);
                        break;
                    case HeadSpace::RST:
                        Close();
                }
            }
    }
}

void Session::HandleTRead()
{
    Fresh();
    try {
        switch (status) {
            case SessionSpace::INIT:
                TcpReadInit();
                break;
            case SessionSpace::CONNECTING:
                throw SessionSpace::ErrorStatus();
            case SessionSpace::PIPE:
                Tcp2Udt();
                break;
            default:
                break;
        }
    }
    catch (TConnect::EAgain) { }
    catch (TConnect::EFin) { SendFin(SessionSpace::TCP2UDT); }
    catch (TConnect::EError) { Rst(); }
}


void Session::HandleTWrite()
{
    Fresh();
    try {
        switch (status) {
            case SessionSpace::INIT:
                throw SessionSpace::ErrorStatus();
            case SessionSpace::CONNECTING:
                status = SessionSpace::PIPE;
                BlockTcp(false);
                break;
            case SessionSpace::PIPE:
                if (direction == SessionSpace::UDT2TCP)
                    UploadWrite();
                else
                    DownloadWrite();
                break;
            default:
                break;
        }
    }
    catch (TConnect::EAgain) { }
    catch (TConnect::EError) { Rst(); }
}

void Session::HandleURead()
{
    Fresh();
    try {
        switch (status) {
            case SessionSpace::INIT:
                //TcpReadInit();
                //break;
            case SessionSpace::CONNECTING:
                //throw SessionSpace::ErrorStatus();
            case SessionSpace::PIPE:
                Udt2Tcp();
                break;
            default:
                break;
        }
    }
    catch (UConnect::EAgain) { }
    catch (UConnect::EFin) { SendFin(SessionSpace::UDT2TCP); }
    catch (UConnect::EError) { Rst(); }
}

void Session::HandleUWrite()
{
    Fresh();
    try {
        switch (status) {
            case SessionSpace::INIT:
                throw SessionSpace::ErrorStatus();
            case SessionSpace::CONNECTING:
                status = SessionSpace::PIPE;
                BlockUdt(false);
                break;
            case SessionSpace::PIPE:
                if (direction == SessionSpace::TCP2UDT)
                    UploadWrite();
                else
                    DownloadWrite();
                break;
            default:
                break;
        }
    }
    catch (UConnect::EAgain) { }
    catch (UConnect::EError) { Rst(); }
}