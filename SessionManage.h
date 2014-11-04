/****************************
  Name:         UdtWrapper
  Author:       Catofes
  Date:         2014-11-1
  License:      All Right Reserved
  Description:  A tunnel to exchange tcp stream via udt.
*****************************/

#ifndef _SESSIONMANAGE_
#define _SESSIONMANAGE_

#include <map>

struct ClientInfo
{
  int uSocket;    //the udt socket num of the connection.
  int sessionId;  //the session id of the same connection.
  bool onread;
  bool onwrite;
};

bool operator < (const ClientInfo &l, const ClientInfo &r);

class SessionManage
{
private:
  map<ClientInfo, int> clientinfo_tsocket;
  map<int, ClientInfo> tsocket_clientinfo;
  int randomSessionId();
  int remove(int tSocket);
public:
  SessionManage();
  int add(int uSocket, int sessionId, int tSocket);
  int rremove(int tSocket);
  int wremove(int tSocket);
  int generate(int uSocket, int tSocket);
  int gettSocket(int uSocket, int sessionId);
  int getuSocket(int tSocket);
  int getSessionId(int tSocket);
};

#endif
