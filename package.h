/****************************
  Name:         UdtWrapper
  Author:       Catofes
  Date:         2014-11-1
  License:      All Right Reserved
  Description:  A tunnel to exchange tcp stream via udt.
*****************************/

#ifndef _PACKAGE_
#define _PACKAGE_

/*
Package head of each transform data. Include info of following data length
and the tcp stream's session id.

If a tcp stream is end, send a package to server/client which length equal
zero. Send with sessionId < 0 while need to reset the udt socket.

*/
struct PackageHead
{
  int sessionId;
  int length;
} __attribute__((packed));

const int PHS = sizeof(PackageHead);
const int BS = 2048000;
const int maxPending = 1024;


#endif
