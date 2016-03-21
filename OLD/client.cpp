/****************************
Name:         UdtWrapper
Author:       Catofes
Date:         2014-11-1
License:      All Right Reserved
Description:  A tunnel to exchange tcp stream via udt.
 *****************************/

#include <iostream>
#include <map>
#include <stdlib.h>
#include <udt/udt.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
using namespace std;

#include "package.h"
#include "SessionManage.h"
#include "tunnel.h"
#include "encrypt.h"
#include "epoll.h"
#include "config.h"


int clientLoop(Config &config, Encrypt &encrypt)
{
	signal(SIGPIPE, SIG_IGN);
	int uSocket,tSocket;
	if((uSocket = udtConnect(config)) < 0 )
	  exit(1);
	if((tSocket = tcpListen(config)) < 0 )
	  exit(1);
	SessionManage manage;
	char buffer[BS];
	int eid = UDT::epoll_create();
	int event = UDT_EPOLL_IN;
	UDT::epoll_add_ssock(eid, tSocket, &event);
	UDT::epoll_add_usock(eid, uSocket);
	set<UDTSOCKET> readfds;
	set<int> sreadfds;
	set<int> swritefds;
	int t = 0;
	timeval lastwake;
	timeval nowwake;
	while(true){
		gettimeofday(&lastwake, 0);
		int ret = UDT::epoll_wait(eid, &readfds, NULL, -1 , &sreadfds, &swritefds);
		gettimeofday(&nowwake,0);
		for(set<UDTSOCKET>::iterator i = readfds.begin(); i != readfds.end(); ++i){
			downloadU2T(eid, uSocket, buffer, manage, encrypt);
		}
		for(set<int>::iterator i = sreadfds.begin(); i != sreadfds.end(); ++i){
			if(*i == tSocket){
				tcpAcpt(eid, tSocket, manage);
			}else{
				uploadT2U(eid, *i, uSocket, buffer, manage, encrypt, config);
				manage.wakeup(*i);
			}
		}
		for(set<int>::iterator i = swritefds.begin(); i != swritefds.end(); ++i){
			downloadB2T(eid, *i, uSocket, manage);
		}
		t = (int)(t * 0.2 + 0.8 * ((nowwake.tv_sec - lastwake.tv_sec) * 1000000 + nowwake.tv_usec - lastwake.tv_usec));
		GabageClean(t, eid, manage, config);
	}
}

int main(int argc, char *argv[])
{
	Encrypt encrypt;
	Config config;
	if(argc < 4) {
		printf("usage: %s listenPort remoteAddress removePort \n",argv[0]);
		exit(1);
	}
	if(string(argv[1]) == "-6"){
		config.remoteAddress = string(argv[3]);
		config.listenPort = atoi(argv[2]);
		config.remotePort = atoi(argv[4]);
		config.IPV6 = true;
		clientLoop(config, encrypt);
	}else{
		config.remoteAddress = string(argv[2]);
		config.listenPort = atoi(argv[1]);
		config.remotePort = atoi(argv[3]);
		clientLoop(config, encrypt);
	}
}
