//
// Created by herbertqiao on 3/24/16.
//

#ifndef UDTWRAPPER_SESSION_HH
#define UDTWRAPPER_SESSION_HH

#include "udtConnection.hh"
#include "tcpConnection.hh"
#include "head.hh"
#include "log.hh"

class SessionManager;

class Session
{
public:
    Session();

    ~Session();

    void HandleTRead();

    void HandleURead();

    void HandleTWrite();

    void HandleUWrite();

    void Rst();

    void Close();

    int GetTime() const
    {
        return active_time;
    }

    int GetId() const
    {
        return session_id;
    }

    SessionSpace::SessionStatus GetStatus() const
    {
        return status;
    }

    friend class SessionManager;

private:
    void TcpReadInit();

    void UdtReadInit();

    void SendFin(SessionSpace::Direction direction);

    void UploadWrite();

    void DownloadWrite();

    void UploadRead();

    void DownloadRead();

    void Tcp2Udt();

    void Udt2Tcp();

    void Fresh();

    void BlockUdt(bool flag);

    void BlockTcp(bool flag);

    void Log(const string &s, char l)
    {
        string str = "Session ";
        str += to_string(session_id);
        str += " :";
        str += s;
        Log::Log(str, l);
    }

    void ArgueClose();

    int session_id;
    int active_time;

    SessionSpace::SessionStatus status = SessionSpace::INIT;
    SessionSpace::Direction direction;

    SessionManager *manager = nullptr;

    udtConnection udt;
    tcpConnection tcp;

    Head head;
    char upload_read_buffer[BS * 2];
    char upload_write_buffer[BS * 2];
    char download_read_buffer[BS * 2];
    char download_write_buffer[BS * 2];

    char buffer[BS];

    uint16_t upload_read_offset = 0;
    uint16_t upload_write_offset = 0;
    uint16_t upload_write_length = 0;
    uint16_t download_read_offset = 0;
    uint16_t download_write_offset = 0;
    uint16_t download_write_length = 0;

    bool upload_fin_flag = false;
    bool download_fin_flag = false;

    bool tproxy = false;

    uint32_t remote_address;
    uint16_t remote_port;
};


#endif //UDTWRAPPER_SESSION_HH
