#include <iostream>
#include <map>
#include <stdlib.h>
#include <udt/udt.h>
#include <signal.h>
using namespace std;

#include "package.h"
#include "SessionManage.h"
#include "tunnel.h"
#include "encrypt.h"
#include "epoll.h"

int serverLoop(int listenPort, string remoteAddress, int remotePort, Encrypt &encrypt, bool IPV6 = false)
{
	signal(SIGPIPE, SIG_IGN);
	int uSocket;
	if((uSocket = udtListen(listenPort, maxPending, IPV6)) < 0 )
	  exit(1);
	SessionManage manage;
	char buffer[BS];
	int eid = UDT::epoll_create();
	UDT::epoll_add_usock(eid, uSocket);
	set<UDTSOCKET> readfds;
	set<int> sreadfds;
	while(true){
		int ret = UDT::epoll_wait(eid, &readfds, NULL, -1 , &sreadfds, NULL);
		for(set<UDTSOCKET>::iterator i = readfds.begin(); i != readfds.end(); ++i){
			if(*i == uSocket){
				udtAcpt(eid, uSocket,IPV6);
			}else{
				uploadU2T(eid, *i, buffer, manage, remoteAddress, remotePort, encrypt);
			}
		}
		for(set<int>::iterator i = sreadfds.begin(); i != sreadfds.end(); ++i){
			downloadT2U(eid, *i, buffer, manage, encrypt);
		}
	}
}

int main(int argc, char *argv[])
{
	Encrypt encrypt;
	if(argc < 4) {
		printf("usage: %s listenPort remoteAddress removePort \n",argv[0]);
		exit(1);
	}
	if(string(argv[1])=="-6"){
		string remoteAddress(argv[3]);
		serverLoop(atoi(argv[2]), remoteAddress, atoi(argv[4]),encrypt, true);
	}else{
		string remoteAddress(argv[2]);
		serverLoop(atoi(argv[1]), remoteAddress, atoi(argv[3]),encrypt);
	}
}
