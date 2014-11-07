/****************************
  Name:         UdtWrapper
  Author:       Catofes
  Date:         2014-11-1
  License:      All Right Reserved
  Description:  A tunnel to exchange tcp stream via udt.
*****************************/

#ifndef _UDTTUNNEL_
#define _UDTTUNNEL_

#include "config.h"

void setTimeOut(int tSocket);
int udtConnect(Config &config);
int udtListen(Config &config);
int tcpConnect(Config &config);
int tcpListen(Config &config);

int udtRecv(int sock, char *buffer, int size);
int udtRecvNoBlock(int sock, char *buffer, int size);
int udtSend(int sock, const char *buffer, int size);
int tcpRecv(int sock, char *buffer, int size);
int tcpSend(int sock, const char *buffer, int size);

#endif
