//
// Created by herbertqiao on 3/24/16.
//

#ifndef UDTWRAPPER_SESSION_HH
#define UDTWRAPPER_SESSION_HH

#include "udtConnection.hh"
#include "tcpConnection.hh"
#include "head.hh"

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

    friend class SessionManager;

private:
    int session_id;
    int active_time;

    SessionSpace::SessionStatus status;
    SessionSpace::Direction direction;

    SessionManager *manager;

    udtConnection udt;
    tcpConnection tcp;

    char upload_buffer[BS];
    char download_buffer[BS];
};


#endif //UDTWRAPPER_SESSION_HH
