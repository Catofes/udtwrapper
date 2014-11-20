/****************************
  Name:         UdtWrapper
  Author:       Catofes
  Date:         2014-11-1
  License:      All Right Reserved
  Description:  A tunnel to exchange tcp stream via udt.
*****************************/

#ifndef _EPOLL_
#define _EPOLL_

#include "config.h"

void signal_callback_handler(int signum);
int udtAcpt(int eid, int uSocket, Config &config);
int tcpAcpt(int eid, int tSocket, SessionManage &manage);
int closeTCP(int eid, int tSocket, SessionManage &manage, bool remove_epoll);
int closeUDT(int eid, int uSocket);
int uploadT2U(int eid, int tSocket, int &uSocket, char* buffer, SessionManage &manage, Encrypt &encrypt, Config &config);
int uploadU2T(int eid, int uSocket, char* buffer, SessionManage &manage, Encrypt &encrypt, Config &config);
int downloadT2U(int eid, int tSocket, char* buffer, SessionManage &manage, Encrypt &encrypt);
int downloadU2T(int eid, int uSocket, char* buffer, SessionManage &manage, Encrypt &encrypt);

#endif
