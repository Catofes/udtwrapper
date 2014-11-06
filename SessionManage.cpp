/****************************
  Name:         UdtWrapper
  Author:       Catofes
  Date:         2014-11-1
  License:      All Right Reserved
  Description:  A tunnel to exchange tcp stream via udt.
*****************************/

#include <iostream>
#include <stdlib.h>
#include <map>
#include <ctime>

using namespace std;

#include "SessionManage.h"

bool operator < (const ClientInfo &l, const ClientInfo &r) {
  if(l.uSocket == r.uSocket)
    return l.sessionId < r.sessionId;
  return l.uSocket < r.uSocket;
}

SessionManage::SessionManage()
{
  srand(time(0));
}

int SessionManage::randomSessionId()
{
  int i = 0;
  while(i<=0)
    i = rand();
  return i;
}

int SessionManage::add(int uSocket, int sessionId, int tSocket)
{
  ClientInfo data;
  data.sessionId = sessionId;
  data.uSocket = uSocket;
  data.onread = true;
  data.onwrite = true;
  clientinfo_tsocket[data]=tSocket;
  tsocket_clientinfo[tSocket]=data;
  return 0;
}

int SessionManage::generate(int uSocket, int tSocket)
{
  ClientInfo data;
  data.uSocket = uSocket;
  data.sessionId = randomSessionId();
  data.onread = true;
  data.onwrite = true;
  while(clientinfo_tsocket.find(data) != clientinfo_tsocket.end())
    data.sessionId = randomSessionId();
  clientinfo_tsocket[data]=tSocket;
  tsocket_clientinfo[tSocket]=data;
  return data.sessionId;
}

int SessionManage::rremove(int tSocket)
{
  if(tsocket_clientinfo.find(tSocket) == tsocket_clientinfo.end())
    return -1;
  tsocket_clientinfo[tSocket].onread = false;
  remove(tSocket);
}

int SessionManage::wremove(int tSocket)
{
  if(tsocket_clientinfo.find(tSocket) == tsocket_clientinfo.end())
    return -1;
  tsocket_clientinfo[tSocket].onwrite = false;
  remove(tSocket);
}

int SessionManage::remove(int tSocket)
{
  if(tsocket_clientinfo.find(tSocket) == tsocket_clientinfo.end())
    return -1;
    if(tsocket_clientinfo[tSocket].onread == false && tsocket_clientinfo[tSocket].onwrite == false){
  clientinfo_tsocket.erase(tsocket_clientinfo.find(tSocket)->second);
  tsocket_clientinfo.erase(tSocket);
}
  return 0;
}

int SessionManage::gettSocket(int uSocket, int sessionId)
{
  ClientInfo data;
  data.sessionId = sessionId;
  data.uSocket = uSocket;
  if(clientinfo_tsocket.find(data) != clientinfo_tsocket.end())
    return clientinfo_tsocket[data];
  return -1;
}

int SessionManage::getSessionId(int tSocket)
{
  if(tsocket_clientinfo.find(tSocket) != tsocket_clientinfo.end())
    return tsocket_clientinfo[tSocket].sessionId;
  return -1;
}

int SessionManage::getuSocket(int tSocket)
{
  if(tsocket_clientinfo.find(tSocket) != tsocket_clientinfo.end())
    return tsocket_clientinfo[tSocket].uSocket;
  return -1;
}

int SessionManage::getsize()
{
	return tsocket_clientinfo.size();
}
