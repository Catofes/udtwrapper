/****************************
	Name:         UdtWrapper
	Author:       Catofes
	Date:         2014-11-1
	License:      All Right Reserved
	Description:  A tunnel to exchange tcp stream via udt.
*****************************/

#define DEBUG
#include <iostream>
#include <signal.h>
#include <udt/udt.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/tcp.h>
#include <string.h>
#include <cmath>
using namespace std;

#include "package.h"
#include "SessionManage.h"
#include "tunnel.h"
#include "encrypt.h"
#include "epoll.h"
#include "config.h"


void signal_callback_handler(int signum){
}

float random1()
{
	return (rand()%100000)/100000.0;
}

int GabageClean(int t, int eid, SessionManage &manage, Config &config)
{
	if(random1() > (t/config.maxT)*(t/config.maxT) || random1() > (manage.getsize()/config.maxS)*(manage.getsize()/config.maxS))
	  return 0;
	int gcNum = rand()%((int)(manage.getsize()*config.p*(t/config.maxT)*(t/config.maxT)*(t/config.maxT)) + 1) + 1;
	int socket=0;
#ifdef DEBUG
	cout<<"[D] Start Garbage Collection. t: " << t << ". Try to collect: "<<gcNum<<" sockets" <<endl;
#endif
	while((socket = manage.cleanone(config)) > 0 && gcNum > 0){
		closeTCP(eid, socket, 2, manage);
		gcNum--;
	}
	return 0;
}

int udtAcpt(int eid, int uSocket, Config &config)
{
	if(config.IPV6){
		struct sockaddr_in6 clientaddr;
		int clien=sizeof(clientaddr);
#ifdef DEBUG
		cout << "[D] Start Accept UDT." << endl;
#endif
		int connfd = UDT::accept(uSocket, (sockaddr *)&clientaddr, &clien);
		setTimeOut(connfd);
		if(connfd < 0) {
			cout << "[E] Can't accept UDT connection." << endl;
			return -1;
		}
		char str[40];
		inet_ntop(AF_INET6, &clientaddr.sin6_addr,str,sizeof(str));
		cout << "[I] Accpet a UDT connection form " << str << ":" << ntohs(clientaddr.sin6_port) << endl;
		UDT::epoll_add_usock(eid, connfd);
		return connfd;

	}else{
		int clien=sizeof(socklen_t);
		struct sockaddr_in clientaddr;
#ifdef DEBUG
		cout << "[D] Start Accept UDT." << endl;
#endif
		int connfd = UDT::accept(uSocket, (sockaddr *)&clientaddr, &clien);
		setTimeOut(connfd);
		if(connfd < 0) {
			cout << "[E] Can't accept UDT connection." << endl;
			return -1;
		}
		char *str = inet_ntoa(clientaddr.sin_addr);
		cout << "[I] Accpet a UDT connection form " << str << ":" << ntohs(clientaddr.sin_port) << endl;
		UDT::epoll_add_usock(eid, connfd);
		return connfd;
	}
}

int tcpAcpt(int eid, int tSocket, SessionManage &manage)
{
	struct sockaddr_in clientaddr;
	socklen_t clilen =  sizeof(struct sockaddr_in);
#ifdef DEBUG
	cout << "[D] Start Accept TCP." << endl;
#endif
	int connfd = accept(tSocket, (sockaddr *)&clientaddr, &clilen);
	if(connfd < 0) {
		cout<<"[E] Can't accept TCP connection."<<errno<<endl;
		return -1;
	}
	char *str = inet_ntoa(clientaddr.sin_addr);
	cout << "[D] Accept a connection from " << str << ":" << ntohs(clientaddr.sin_port) << endl;
	//Set send timeout
	unsigned int timeout = 10;
	if (-1 == setsockopt(connfd, IPPROTO_TCP, TCP_USER_TIMEOUT, &timeout, sizeof(timeout))) {
		return -1;
	}
	int keep_alive = 1;
	int keep_idle = 5, keep_interval = 1, keep_count = 3;
	int ret = 0;
	if (-1 == (ret = setsockopt(connfd, SOL_SOCKET, SO_KEEPALIVE, &keep_alive, sizeof(keep_alive)))) {
		return -1;
	}
	if (-1 == (ret = setsockopt(connfd, IPPROTO_TCP, TCP_KEEPIDLE, &keep_idle,	sizeof(keep_idle)))) {
		return -1;
	}
	if (-1 == (ret = setsockopt(connfd, IPPROTO_TCP, TCP_KEEPINTVL, &keep_interval,	sizeof(keep_interval)))) {
		return -1;
	}
	if (-1 == (ret = setsockopt(connfd, IPPROTO_TCP, TCP_KEEPCNT, &keep_count,	sizeof(keep_count)))) {
		return -1;
	}
	manage.generate(0, connfd);
	SetSocketBlockingEnabled(connfd, false);
	int event = UDT_EPOLL_IN;
	UDT::epoll_add_ssock(eid, connfd, &event );
	return connfd;
}

int closeTCP(int eid, int tSocket, int num, SessionManage &manage)
{
	if(manage.getSessionId(tSocket) > 0){
		if(num == 0)
		  manage.rremove(tSocket);
		else if(num == 1)
		  manage.wremove(tSocket);
		else{
			manage.rremove(tSocket);
			manage.wremove(tSocket);
		}
	}
	if(num != 1)
	  UDT::epoll_remove_ssock(eid, tSocket);
#ifdef DEBUG
	cout<<"[D] Close TCP:"<<tSocket<<endl;
#endif
	shutdown(tSocket, num);
	if(manage.getSessionId(tSocket) <= 0)
	  close(tSocket);
	return 0;
}

int closeUDT(int eid, int uSocket, SessionManage &mange)
{
#ifdef DEBUG
	cout<<"[D] Close UDT:"<<uSocket<<endl;
#endif
	UDT::epoll_remove_usock(eid, uSocket);
	//UDT::close(uSocket);
	return 0;
}


int uploadT2U(int eid, int tSocket, int &uSocket, char* buffer, SessionManage &manage, Encrypt &encrypt, Config &config)
{
#ifdef DEBUG
	cout<<"[D] UploadT2U SIG UP."<<endl;
#endif
	int size = recv(tSocket, &buffer[PHS], BS-PHS, 0);
	PackageHead * head = (PackageHead *) buffer;

	//Check if the tSocket is closed. This things SHOULD NEVER happend.
	if((head->sessionId = manage.getSessionId(tSocket)) < 0){
		cout<<"[B] Closed Socket get data."<<endl;
		closeTCP(eid, tSocket, 2, manage);
		return 0;
	}

	if(size == -1 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR))
	  return 0;
	
	if(size == -1){
#ifdef DEBUG
	  cout<<"[D] Get Data error at Code: "<<errno<<endl;
#endif
}

	//if TCP recv <= 0 . Mean TCP Close. Send disconnect package. And remove listen.
	if(size <= 0){
		size = 0;
	}
	head->length = size;

	//Encrypt Data.
	encrypt.encrypt(buffer + PHS, size);

	//Send Data.
#ifdef DEBUG
	cout<<"[D] UploadT2U send. Size:"<<size<<endl;
#endif
	int send = udtSend(uSocket, buffer, size + PHS);

	if(size == 0)
	  closeTCP(eid, tSocket, 0, manage);
	//manage.rremove(tSocket);

	if(send == UDT::ERROR)
	{
		cout << "[E] UDT Connection lost." << endl;
		//prepare to reconnect.
		UDT::close(uSocket);
		UDT::epoll_remove_usock(eid, uSocket);
		UDT::epoll_remove_ssock(eid, tSocket);
		shutdown(tSocket, 2);
		int newuSocket = 0;
		while(newuSocket <= 0)
		{
			cout<<"[E] Try to Reconnect to Server." <<endl;
			sleep(3);
			newuSocket = udtConnect(config);
		}
		uSocket = newuSocket;
		UDT::epoll_add_usock(eid, uSocket);
	}

	return send;
}

int uploadU2T(int eid, int uSocket, char* buffer, SessionManage &manage, Encrypt &encrypt, Config &config)
{
	PackageHead * head = (PackageHead*) buffer;
#ifdef DEBUG
	cout<<"[D] UploadU2T recv SIG UP."<<endl;
#endif
	//Read PackageHead.

	int receivebytes = udtRecvNoBlock(eid, uSocket, buffer, PHS);
	//int receivebytes = udtRecv(uSocket, buffer, PHS);
	//check error
	if(receivebytes == -2)
	{
		return 0;
	}

	if(receivebytes == -1 ){
		cout<<"[E] UDT ERROR.Socks:"<<uSocket<<"	At code:"<<UDT::getlasterror_code()<<endl;
		UDT::getlasterror().clear();
		closeUDT(eid, uSocket, manage);
		return 0;
	}

	int tSocket;
	//If sessionId is new, Setup new tcp connection.
	if((tSocket = manage.gettSocket(uSocket, head->sessionId)) == -1){
		tSocket = tcpConnect(config);
		setTimeOut(tSocket);
		UDT::epoll_add_ssock(eid, tSocket);
		manage.add(uSocket, head->sessionId, tSocket);
#ifdef DEBUG
		cout<<"[D] SessionManage Size:"<<manage.getsize()<<endl;
#endif
	}

	//Reset
	if(head->length == -1){
#ifdef DEBUG
		cout<<"[D] Reset SIG received." <<endl;
#endif
		closeTCP(eid, tSocket, 2,manage);
		return 0;
	}

	if(head->length <= -10000){
#ifdef DEBUG
		cout<<"[D] Slow Down SIG received. Buffersize: "<<head->length<<" Socket:"<<tSocket<<endl;
#endif
		int slowbuffer = -10000-head->length;
		
		ClientInfo *info = &(manage.tsocket_clientinfo[tSocket]);
		if(random1() <= sqrt(slowbuffer/(config.maxSlowBuffer+0.1)) ){
			if(info->onsleep == false){
#ifdef DEBUG
				cout<<"[D] socket sleep. Socket: "<<tSocket<<endl;
#endif
				info->onsleep = true;
				UDT::epoll_remove_ssock(eid, tSocket);
			}
		}else{
		  if(info->onsleep == true){
			  info->onsleep = false;
#ifdef DEBUG
			  cout<<"[D] socket wakeup."<<endl;
#endif
			  UDT::epoll_add_ssock(eid, tSocket);
		  }
		}
		return 0;
	}

	//If length = 0 . Means Stop Write on a TCP.
	if(head->length == 0||head->length > BS-PHS){
		//shutdown(tSocket,1);
		//manage.wremove(tSocket);
		closeTCP(eid, tSocket, 1, manage);
		return 0;
	}

	//Receive Data.
	udtRecv(uSocket, buffer + PHS, head->length);

	//Decrypt Data.
	int size = head->length;
	encrypt.decrypt(buffer + PHS,size);
#ifdef DEBUG
	cout<<"[D] UploadU2T send. Size:"<<size<<endl;
#endif

	//Send Data;
	return tcpSend(tSocket, buffer + PHS, size);
}

int downloadT2U(int eid, int tSocket, char* buffer, SessionManage &manage, Encrypt &encrypt)
{
#ifdef DEBUG
	cout<<"[D] DownloadT2U SIG UP."<<endl;
#endif
	int size = recv(tSocket, &buffer[PHS], BS-PHS, 0);
	PackageHead * head = (PackageHead*) buffer;

	//Check if the tSocket is closed. This things SHOULD NEVER happend.
	if((head->sessionId = manage.getSessionId(tSocket)) < 0){
		cout<<"[B] Closed Socket get data."<<endl;
		closeTCP(eid, tSocket, 2, manage);
		return 0;
	}

	//if TCP recv <= 0 . Mean TCP Close. Send disconnect package. And remove listen.
	if(size <= 0){
		size = 0;
	}
	head->length = size;

	//Encrypt Data.
	encrypt.encrypt(buffer + PHS, size);

	//Send Data.
#ifdef DEBUG
	cout<<"[D] DownloadT2U send. Size:"<<size<<endl;
#endif
	int uSocket = manage.getuSocket(tSocket);
	int send = udtSend(uSocket, buffer, size + PHS);

	if(send == UDT::ERROR){
		cout<<"[E] Connect Lost at:"<<uSocket<<endl;
		UDT::epoll_remove_usock(eid, uSocket);
		UDT::close(uSocket);
		closeTCP(eid, tSocket, 2, manage);
	}

	if(size == 0)
	  closeTCP(eid, tSocket, 0, manage);
	//manage.rremove(tSocket);

	return send;
}

int downloadU2T(int eid, int uSocket, char* buffer, SessionManage &manage, Encrypt &encrypt)
{
	PackageHead * head = (PackageHead*) buffer;
#ifdef DEBUG
	cout<<"[D] DownloadU2T recv SIG UP." <<endl;
#endif
	//Read PackageHead.
	//Set UDT_RECVTIMEO
	//int timeout = 1;
	//UDT::setsockopt(uSocket, 0, UDT_RCVTIMEO, &timeout, sizeof(int));
	int receivebyte = udtRecvNoBlock(eid, uSocket, buffer, PHS);

	if(receivebyte < 0){
#ifdef DEBUG
		cout<<"Error Sig Happend"<<endl;
#endif
		return 0;
	}
	//timeout = -1;
	//UDT::setsockopt(uSocket, 0, UDT_RCVTIMEO, &timeout, sizeof(int));
	//No tSocket get. Client get a error Data. Remove the Data. Send RESET SIG.
	int tSocket;
	if((tSocket = manage.gettSocket(0, head->sessionId)) < 0){
		udtRecv(uSocket, buffer + PHS, head->length);
		head->length = -1;
		udtSend(uSocket, buffer, PHS);
		return 0;
	}

	//If length = 0 . Stop Write.
	if(head->length == 0||head->length > BS-PHS){
		//shutdown(tSocket,1);
		//manage.wremove(tSocket);
		closeTCP(eid, tSocket, 1, manage);
		return 0;
	}

	//Receive Data.
	udtRecv(uSocket, buffer + PHS, head->length);

	//Decrypt Data.
	int size = head->length;
	encrypt.decrypt(buffer + PHS, size);
	//save data to socket's buffer, send info to server .
	ClientInfo *info = &(manage.tsocket_clientinfo.find(tSocket)->second);

	int sendsize = 0;
	if(info->sendblock == false){
		//Send Data
		sendsize = tcpSendNoBlock(tSocket, buffer+PHS, size);
		if(sendsize == -1){
			cout<<"[E] Tcp error. Close TCP. Error at: "<<errno<<endl;
			closeTCP(eid, tSocket, 2, manage);
			return -1;
		}
		if (sendsize == size){
#ifdef DEBUG
			cout<<"[D] DownloadU2T send. Size:"<<size<<endl;
#endif
			return sendsize;
		}
	}
	if(sendsize < 0)
	  sendsize = 0;

	BufferInfo newbuffer;
	newbuffer.size = size - sendsize;
	newbuffer.buffer = new char[size - sendsize];
	newbuffer.offset = 0;
	memcpy(newbuffer.buffer, buffer + PHS + sendsize, size - sendsize);
	info->buffers.push_back(newbuffer);
	info->size += (size - sendsize);
	if(random1()<0.2){
		head->length = -10000 - info->size;
		udtSend(uSocket,buffer, PHS);
	}
	info->sendblock = true;
#ifdef DEBUG
	cout<<"[D] Tcp buffer data. Socket: "<<tSocket<<" Size: "<<size<<" Sendsize: "<<sendsize<<" Totalsize: "<<info->size<<endl;
#endif
	UDT::epoll_remove_ssock(eid, tSocket);
	int event = UDT_EPOLL_IN | UDT_EPOLL_OUT;
	UDT::epoll_add_ssock(eid, tSocket, &event);
	//return tcpSend(tSocket, buffer + PHS, size);
}

int downloadB2T(int eid, int tSocket, int uSocket, SessionManage &manage)
{
#ifdef DEBUG
	cout<<"[D] DownloadB2T SIG UP."<<endl;
#endif
	int sendsize = 0;
	if(manage.tsocket_clientinfo.find(tSocket) == manage.tsocket_clientinfo.end())
	{
		UDT::epoll_remove_ssock(eid,tSocket);
		int event = UDT_EPOLL_IN;
		UDT::epoll_add_ssock(eid,tSocket, &event);
	}
	ClientInfo *info = &(manage.tsocket_clientinfo.find(tSocket)->second);

	if(info->buffers.size()== 0 || info->sendblock == false){
		info->sendblock = false;
		UDT::epoll_remove_ssock(eid,tSocket);
		int event = UDT_EPOLL_IN;
		UDT::epoll_add_ssock(eid,tSocket, &event);
		return 0;
	}
	BufferInfo * buffer = &(info->buffers[0]);
	sendsize = tcpSendNoBlock(tSocket, buffer->buffer + buffer->offset, buffer->size - buffer->offset);
#ifdef DEBUG
	cout<<"[D] DownloadB2T Send: "<<sendsize<<endl;
#endif
	if(sendsize == buffer->size - buffer->offset){
		delete buffer->buffer;
		info->buffers.erase(info->buffers.begin());
		info->size -= sendsize;
		return 0;
	}

	if(sendsize == -1 && (!(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)))
	{
#ifdef DEBUG
		cout<<"[D] DownloadB2T TCP Broken. At:"<<errno<<endl;
#endif
		PackageHead head;
		head.length = -1;
		head.sessionId = info->sessionId;
		udtSend(uSocket,(char *) &head, PHS);
		for(int i=0;i<info->buffers.size();i++)
		  delete info->buffers[i].buffer;
		closeTCP(eid, tSocket, 2, manage);
		return -1;
	}
	if(sendsize < 0)
	  sendsize = 0;
	buffer->offset += sendsize;
	info->size -= sendsize;
	if(info->size < 1000000){
		PackageHead head;
		head.length = -10000 - info->size;
		head.sessionId = info->sessionId;
		udtSend(uSocket,(char *) &head, PHS); 
	}
	return 0;
}
