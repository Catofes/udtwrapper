//
// Created by herbertqiao on 3/24/16.
//

#include "sessionManager.hh"
#include "error.hh"
#include "random"

SessionManager::SessionManager()
{
    session_upper = 0;
}

SessionManager::~SessionManager()
{
    for (const auto &n: session_store) {
        if (n.second != nullptr) {
            delete n.second;
        }
    }
    session_store.clear();
    tcp2session.clear();
    udt2session.clear();
}

Session *SessionManager::CreateSessionByTcp(int socket)
{
    Session *s = new Session();
    s->remote_address = remote_address;
    s->remote_port = remote_port;
    s->tcp.SetSocket(socket);
    s->tcp.SetStatus(Connection::PIPE);
    s->session_id = session_upper;
    s->manager = this;
    s->direction = SessionSpace::TCP2UDT;
    s->status = SessionSpace::INIT;
    s->udt.SetEvent(EPOLLOpt::UDT_EPOLL_IN | EPOLLOpt::UDT_EPOLL_ERR);

    session_store[s->session_id] = s;
    tcp2session[socket] = s->session_id;

    session_upper++;
    return s;
}

Session *SessionManager::CreateSessionByUdt(int socket)
{
    Session *s = new Session();
    s->remote_address = remote_address;
    s->remote_port = remote_port;
    s->udt.SetSocket(socket);
    s->udt.SetStatus(Connection::PIPE);
    s->session_id = session_upper;
    s->manager = this;
    s->direction = SessionSpace::UDT2TCP;
    s->status = SessionSpace::INIT;
    s->udt.SetEvent(EPOLLOpt::UDT_EPOLL_IN | EPOLLOpt::UDT_EPOLL_ERR);

    session_store[s->session_id] = s;
    udt2session[socket] = s->session_id;

    session_upper++;
    return s;
}

Session *SessionManager::GetSessionByTcp(int socket)
{
    auto session_id = tcp2session.find(socket);
    if (session_id == tcp2session.end())
        throw SManager::NotFound();
    auto session = session_store.find(session_id->second);
    if (session == session_store.end())
        throw SManager::NotFound();
    return session->second;
}

Session *SessionManager::GetSessionByUdt(int socket)
{
    auto session_id = udt2session.find(socket);
    if (session_id == udt2session.end())
        throw SManager::NotFound();
    auto session = session_store.find(session_id->second);
    if (session == session_store.end())
        throw SManager::NotFound();
    return session->second;
}

uint32_t SessionManager::GetSessionCount()
{
    return (uint32_t) session_store.size();
}

void SessionManager::GarbageCollection()
{
    int t = (int) time(NULL);
    uint32_t s = GetSessionCount();
    if (s > gc_limit || (std::rand() % 100000) / 100000. < gc_prob * (s - gc_start_size)) {
        Log::Log("Start GC.", 3);
        vector<int> remove_session_ids;
        for (std::pair<int, Session *> u:session_store) {
            if (t - u.second->GetTime() > 600) {
                u.second->Close();
                delete u;
                remove_session_ids.push_back(u.first);
            }
            else if (u.second->GetStatus() == SessionSpace::CLOSE) {
                u.second->Close();
                delete u;
                remove_session_ids.push_back(u.first);
            }
            if (remove_session_ids.size() > gc_max * GetSessionCount())
                break;
        }
        for (int u:remove_session_ids) {
            session_store.erase(u);
        }

    }
}

void SessionManager::SetEpoll(int epoll_id)
{
    epoll = epoll_id;
}