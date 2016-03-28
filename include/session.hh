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

    inline int GetId()
    {
        return session_id;
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

    void Close();

    void Fresh();

    void BlockUdt(bool flag);

    void BlockTcp(bool flag);

    inline void Log(string s, char l)
    {
        string str = "Session ";
        str += session_id;
        str += " :";
        str += s;
        Log::Log(str, l);
    }

    int session_id;
    int active_time;

    SessionSpace::SessionStatus status = SessionSpace::INIT;
    SessionSpace::Direction direction;

    SessionManager *manager = nullptr;

    udtConnection udt;
    tcpConnection tcp;

    char upload_read_buffer[BS * 2];
    char upload_write_buffer[BS * 2];
    char download_read_buffer[BS * 2];
    char download_write_buffer[BS * 2];

    char buffer[BS];

    uint16_t upload_read_offset;
    uint16_t upload_write_offset;
    uint16_t upload_write_length;
    uint16_t download_read_offset;
    uint16_t download_write_offset;
    uint16_t download_write_length;

    bool upload_fin_flag = false;
    bool download_fin_flag = false;

    bool tproxy = false;

    uint32_t remote_address;
    uint16_t remote_port;
};


#endif //UDTWRAPPER_SESSION_HH
