/****************************
  Name:         UdtWrapper
  Author:       Catofes
  Date:         2014-11-1
  License:      All Right Reserved
  Description:  A tunnel to exchange tcp stream via udt.
*****************************/

#ifndef _UDTTUNNEL_
#define _UDTTUNNEL_
void setTimeOut(int tSocket);
int udtConnect(string remoteAddress, int portNum);
int udtListen(int portNum, int maxPending);
int tcpConnect(string remoteAddress, int portNum);
int tcpListen(int portNum, int maxPending);

int udtRecv(int sock, char *buffer, int size);
int udtRecvNoBlock(int sock, char *buffer, int size);
int udtSend(int sock, const char *buffer, int size);
int tcpRecv(int sock, char *buffer, int size);
int tcpSend(int sock, const char *buffer, int size);

#endif
