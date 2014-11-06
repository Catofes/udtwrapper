#ifndef _EPOLL_
#define _EPOLL_
void signal_callback_handler(int signum);
int udtAcpt(int eid, int uSocket,bool IPV6);
int tcpAcpt(int eid, int tSocket, SessionManage &manage);
int closeTCP(int eid, int tSocket, SessionManage &manage);
int closeUDT(int eid, int uSocket);
int uploadT2U(int eid, int tSocket, int uSocket, char* buffer, SessionManage &manage, Encrypt &encrypt);
int uploadU2T(int eid, int uSocket, char* buffer, SessionManage &manage, string remoteAddress, int portNum, Encrypt &encrypt);
int downloadT2U(int eid, int tSocket, char* buffer, SessionManage &manage, Encrypt &encrypt);
int downloadU2T(int eid, int uSocket, char* buffer, SessionManage &manage, Encrypt &encrypt);

#endif
