//
// Created by herbertqiao on 3/24/16.
//

#ifndef UDTWRAPPER_SESSIONMANAGER_HH
#define UDTWRAPPER_SESSIONMANAGER_HH

#include "sessionManager.hh"
#include "session.hh"
#include <unordered_map>

using std::unordered_map;

class SessionManager
{
public:

    SessionManager();

    ~SessionManager();

    void GarbageCollection();

    Session *GetSessionByTcp(int socket);

    Session *GetSessionByUdt(int socket);

    Session *CreateSessionByTcp(int socket);

    Session *CreateSessionByUdt(int socket);

    uint32_t GetSessionCount();

    void SetEpoll(int epoll_id);

    friend class UEpoll;

private:
    int session_upper;
    int epoll;

    string remote_address;
    uint16_t remote_port;

    unordered_map<int, Session *> session_store;
    unordered_map<int, int> tcp2session;
    unordered_map<int, int> udt2session;
};


#endif //UDTWRAPPER_SESSIONMANAGER_HH
