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

    int GetEpoll() const
    {
        return epoll;
    }

    friend class UEpoll;

    friend class Session;

private:
    int session_upper;
    int epoll;

    uint32_t remote_address;
    uint16_t remote_port;

    unordered_map<int, Session *> session_store;
    unordered_map<int, int> tcp2session;
    unordered_map<int, int> udt2session;

    const uint16_t gc_start_size = 100;
    const double gc_prob = 0.0003;
    const double gc_max = 0.1;
    const uint16_t gc_limit = 1000;

};


#endif //UDTWRAPPER_SESSIONMANAGER_HH
