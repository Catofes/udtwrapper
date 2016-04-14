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
        manager->tcp2session.erase(tcp.GetSocket());
        tcp.Close();
    }
    if (udt.GetSocket() > 0) {
        UDT::epoll_remove_usock(manager->GetEpoll(), udt.GetSocket());
        manager->udt2session.erase(udt.GetSocket());
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
        if (tcp.status == Connection::BLOCKING)
            return;
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
        if (tcp.status == Connection::PIPE)
            return;
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
        if (udt.status == Connection::BLOCKING)
            return;
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
        if (udt.status == Connection::PIPE)
            return;
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
                if (upload_write_length + upload_write_offset > BS)
                    BlockUdt(true);
                else
                    BlockUdt(false);
                break;
            case SessionSpace::UDT2TCP:
                s = tcp.Write(data, upload_write_length);
                upload_write_offset += s;
                upload_write_length -= s;
                if (upload_write_length == 0)
                    upload_write_offset = 0;
                if (upload_write_length + upload_write_offset > BS)
                    BlockTcp(true);
                else
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
                if (download_write_length + download_write_offset > BS)
                    BlockUdt(true);
                else
                    BlockUdt(false);
                break;
            case SessionSpace::TCP2UDT:
                s = tcp.Write(data, download_write_length);
                download_write_offset += s;
                download_write_length -= s;
                if (download_write_length == 0)
                    download_write_offset = 0;
                if (download_write_length + download_write_offset > BS)
                    BlockTcp(true);
                else
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
    switch (direction) {
        case SessionSpace::TCP2UDT:
            upload_read_offset += tcp.Read(upload_read_buffer + upload_read_offset, DS - upload_read_offset);
            break;
        case SessionSpace::UDT2TCP:
            upload_read_offset += udt.Read(upload_read_buffer + upload_read_offset, BS - upload_read_offset);
            break;
    }
}

void Session::DownloadRead()
{
    Log("Download Read.", 1);
    switch (direction) {
        case SessionSpace::UDT2TCP:
            download_read_offset += tcp.Read(download_read_buffer + download_read_offset, DS - download_read_offset);
            break;
        case SessionSpace::TCP2UDT:
            download_read_offset += udt.Read(download_read_buffer + download_read_offset, BS - download_read_offset);
            break;
    }
}

void Session::SendFin(SessionSpace::Direction d)
{
    switch (d) {
        case SessionSpace::TCP2UDT:
            Log("Send Fin to UDT.", 1);

            tcp.event = tcp.event & (EPOLLOpt::UDT_EPOLL_OUT | EPOLLOpt::UDT_EPOLL_ERR);
            UDT::epoll_remove_ssock(manager->epoll, tcp.tcp_socket);
            if (tcp.event != 0)
                UDT::epoll_add_ssock(manager->epoll, tcp.tcp_socket, &tcp.event);

            head.type = HeadSpace::FIN;
            if (direction == SessionSpace::TCP2UDT) {
                upload_write_length += head.Pack(upload_write_buffer + upload_write_offset + upload_write_length);
                UploadWrite();
                upload_fin_flag = true;
            }
            else {
                download_write_length += head.Pack(
                        download_write_buffer + download_write_offset + download_write_length);
                DownloadWrite();
                download_fin_flag = true;
            }
            udt.SendFin();
            break;
        case SessionSpace::UDT2TCP:
            Log("Send Fin to TCP.", 1);

            udt.event = udt.event & (EPOLLOpt::UDT_EPOLL_OUT | EPOLLOpt::UDT_EPOLL_ERR);
            UDT::epoll_remove_usock(manager->epoll, udt.udt_socket);
            UDT::epoll_add_usock(manager->epoll, udt.udt_socket, &udt.event);

            if (tcp.status == Connection::PIPE)
                tcp.SendFin();
            else {
                tcp.SetFin();
            }
            if (direction == SessionSpace::TCP2UDT)
                download_fin_flag = true;
            else
                upload_fin_flag = true;
            break;
    }
    ArgueClose();
}

void Session::ArgueClose()
{
    if (upload_fin_flag && download_fin_flag)
        Close();
}

void Session::TcpReadInit()
{
    Log("Tcp Read Init.", 1);
    if (direction == SessionSpace::UDT2TCP)
        throw SessionSpace::ErrorStatus();
    udt.Connect(remote_address, remote_port);
    manager->udt2session[udt.udt_socket] = this->session_id;
    BlockUdt(true);
    status = SessionSpace::CONNECTING;
}

void Session::UdtReadInit()
{
    Log("Udt Read Init.", 1);
    if (direction == SessionSpace::TCP2UDT)
        throw SessionSpace::ErrorStatus();
    tcp.Connect(remote_address, remote_port);
    manager->tcp2session[tcp.tcp_socket] = this->session_id;
    BlockTcp(true);
    status = SessionSpace::CONNECTING;
}


void Session::Tcp2Udt()
{
    switch (direction) {
        case SessionSpace::TCP2UDT:
            if (udt.fin)
                throw SessionSpace::SendData2Fin();
            switch (status) {
                case SessionSpace::INIT:
                    throw SessionSpace::ErrorStatus();
                    break;
                case SessionSpace::CONNECTING:
                    Log("Send Connect Command.", 1);
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
                case SessionSpace::CLOSE:
                    this->Close();
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
    switch (direction) {
        case SessionSpace::TCP2UDT:
            try {
                DownloadRead();
            } catch (UConnect::EAgain) { }
            while (download_read_offset > 0) {
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
                        }
                        download_write_length += head.data_length;
                        download_read_offset -= (head.data_length + 3);
                        DownloadWrite();
                        break;
                    case HeadSpace::CONNECT:
                        throw HeadSpace::UnknownType();
                    case HeadSpace::FIN:
                        SendFin(SessionSpace::UDT2TCP);
                        download_read_offset = 0;
                        break;
                    case HeadSpace::RST:
                        Close();
                }
            }
            break;
        case SessionSpace::UDT2TCP:
            try {
                UploadRead();
            } catch (UConnect::EAgain) { }
            bool f = true;
            while (upload_read_offset > 0 && f) {
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
                        }
                        upload_write_length += head.data_length;
                        upload_read_offset -= (head.data_length + 3);
                        UploadWrite();
                        break;
                    case HeadSpace::CONNECT:
                        Log("Got Connect Command.", 1);
                        if (tproxy) {
                            remote_address = head.address;
                            remote_port = head.port;
                        }
                        if (7 < upload_read_offset) {
                            memcpy(buffer, upload_read_buffer + 7, upload_read_offset - 7);
                            memcpy(upload_read_buffer, buffer, upload_read_offset - 7);
                        }
                        upload_read_offset -= 7;
                        UdtReadInit();
                        //Get out of loop.
                        f = false;
                        break;
                    case HeadSpace::FIN:
                        SendFin(SessionSpace::UDT2TCP);
                        upload_read_offset = 0;
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
    Log("TCP Read.", 1);
    try {
        switch (status) {
            case SessionSpace::INIT:
                TcpReadInit();
                break;
            case SessionSpace::CONNECTING:
                if (direction == SessionSpace::UDT2TCP)
                    tcp.CheckWrite();
                else
                    BlockTcp(true);
                break;
            case SessionSpace::PIPE:
                Tcp2Udt();
                break;
            case SessionSpace::CLOSE:
                Close();
                break;
        }
    }
    catch (TConnect::EAgain) { Log("EAgain", 0); }
    catch (TConnect::EFin) { SendFin(SessionSpace::TCP2UDT); }
    catch (TConnect::EError) { Rst(); }
}


void Session::HandleTWrite()
{
    Fresh();
    Log("TCP Write.", 1);
    try {
        switch (status) {
            case SessionSpace::INIT:
                throw SessionSpace::ErrorStatus();
            case SessionSpace::CONNECTING:
                tcp.CheckWrite();
                status = SessionSpace::PIPE;
                BlockTcp(false);
                Udt2Tcp();
                break;
            case SessionSpace::PIPE:
                if (direction == SessionSpace::UDT2TCP)
                    UploadWrite();
                else
                    DownloadWrite();
                break;
            case SessionSpace::CLOSE:
                Close();
                break;
        }
    }
    catch (TConnect::EAgain) { }
    catch (TConnect::EError) { Rst(); }
}

void Session::HandleURead()
{
    Fresh();
    Log("UDT Read.", 1);
    try {
        switch (status) {
            case SessionSpace::INIT:
            case SessionSpace::CONNECTING:
            case SessionSpace::PIPE:
                Udt2Tcp();
                break;
            case SessionSpace::CLOSE:
                Close();
                break;
        }
    }
    catch (HeadSpace::Partial) { }
    catch (UConnect::EAgain) { }
    catch (UConnect::EFin) { SendFin(SessionSpace::UDT2TCP); }
    catch (UConnect::EError) { Rst(); }
}

void Session::HandleUWrite()
{
    Fresh();
    Log("UDT Write.", 1);
    try {
        switch (status) {
            case SessionSpace::INIT:
                throw SessionSpace::ErrorStatus();
            case SessionSpace::CONNECTING:
                Tcp2Udt();
                status = SessionSpace::PIPE;
                break;
            case SessionSpace::PIPE:
                if (direction == SessionSpace::TCP2UDT)
                    UploadWrite();
                else
                    DownloadWrite();
                break;
            case SessionSpace::CLOSE:
                Close();
                break;
        }
    }
    catch (UConnect::EAgain) { }
    catch (UConnect::EError) { Rst(); }
}