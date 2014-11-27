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
#include <vector>
#include "config.h"

struct BufferInfo
{
	int size;
	int offset;
	char * buffer;
};

struct ClientInfo
{
  int uSocket;    //the udt socket num of the connection.
  int sessionId;  //the session id of the same connection.
  int lastAck;
  bool onread;
  bool onwrite;
  bool sendblock;
  int size;
  bool onsleep;
  vector<BufferInfo> buffers;
};

bool operator < (const ClientInfo &l, const ClientInfo &r);

class SessionManage
{
private:
  map<ClientInfo, int> clientinfo_tsocket;
  int randomSessionId();
  int remove(int tSocket);
  int gettime();
public:
  map<int, ClientInfo> tsocket_clientinfo;
  SessionManage();
  int add(int uSocket, int sessionId, int tSocket);
  int wakeup(int tSocket);
  int cleanone(Config &config);
  int rremove(int tSocket);
  int wremove(int tSocket);
  int generate(int uSocket, int tSocket);
  int gettSocket(int uSocket, int sessionId);
  int getuSocket(int tSocket);
  int getSessionId(int tSocket);
  int getsize();
};

#endif
