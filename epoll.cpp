#include <iostream>
#include <signal.h>
#include <udt/udt.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
using namespace std;

#include "package.h"
#include "SessionManage.h"
#include "tunnel.h"
#include "encrypt.h"
#include "epoll.h"

void signal_callback_handler(int signum){
}

int udtAcpt(int eid, int uSocket)
{
	int clien=sizeof(socklen_t);
	struct sockaddr_in clientaddr;
	int connfd = UDT::accept(uSocket, (sockaddr *)&clientaddr, &clien);
	if(connfd < 0) {
		cout << "[E] Can't accept UDT connection." << endl;
		return -1;
	}
	char *str = inet_ntoa(clientaddr.sin_addr);
	cout << "[I] Accpet a UDT connection form " << str << ":" << ntohs(clientaddr.sin_port) << endl;
	UDT::epoll_add_usock(eid, connfd);
	return connfd;
}

int tcpAcpt(int eid, int tSocket, SessionManage &manage)
{
	struct sockaddr_in clientaddr;
  socklen_t clilen =  sizeof(struct sockaddr_in);
	int connfd = accept(tSocket, (sockaddr *)&clientaddr, &clilen);
	if(connfd < 0) {
		cout<<"[E] Can't accept TCP connection."<<endl;
		return -1;
	}
	char *str = inet_ntoa(clientaddr.sin_addr);
	cout << "Accept a connection from " << str << ":" << ntohs(clientaddr.sin_port)<<endl;
	manage.generate(0, connfd);
	UDT::epoll_add_ssock(eid, connfd);
	return connfd;
}

int closeTCP(int eid, int tSocket, int num, SessionManage &manage)
{
	if(manage.getSessionId(tSocket) > 0){
		manage.remove(tSocket);
		UDT::epoll_remove_ssock(eid, tSocket);
		shutdown(tSocket, num);
	}
	return 0;
}

int closeUDT(int eid, int uSocket)
{
	UDT::epoll_remove_usock(eid, uSocket);
	UDT::close(uSocket);
	return 0;
}

int uploadT2U(int eid, int tSocket, int uSocket, char* buffer, SessionManage &manage, Encrypt &encrypt)
{
	int size = recv(tSocket, &buffer[PHS], BS-PHS, 0);
	if(size >= 0){
		PackageHead * head = (PackageHead *) buffer;
		if((head->sessionId = manage.getSessionId(tSocket)) > 0){
			encrypt.encrypt(buffer + PHS, size);
			head->length = size;
			int send = udtSend(uSocket, buffer, size + PHS);
      if(size == 0)
        UDT::epoll_remove_ssock(eid, tSocket);
			return send;
		}
	}
	closeTCP(eid, tSocket, 2, manage);
	return 0;
}

int uploadU2T(int eid, int uSocket, char* buffer, SessionManage &manage, string remoteAddress, int portNum, Encrypt &encrypt)
{
	PackageHead * head = (PackageHead*) buffer;
	if(udtRecv(uSocket, buffer, PHS) == PHS){
		int tSocket;
		if((tSocket = manage.gettSocket(uSocket, head->sessionId)) == -1){
			tSocket = tcpConnect(remoteAddress, portNum);
			UDT::epoll_add_ssock(eid, tSocket);
			manage.add(uSocket, head->sessionId, tSocket);
		}
		if(head->length == 0||head->length > BS-PHS){
			closeTCP(eid, tSocket, 1, manage);
			return 0;
		}
		if(udtRecv(uSocket, buffer + PHS, head->length) == head->length){
			int size = head->length;
			encrypt.decrypt(buffer + PHS,size);
			return tcpSend(tSocket, buffer + PHS, size);
		}
	}
	closeUDT(eid, uSocket);
	return -1;
}

int downloadT2U(int eid, int tSocket, char* buffer, SessionManage &manage, Encrypt &encrypt)
{
	int size = recv(tSocket, &buffer[PHS], BS-PHS, 0);
	if(size >= 0){
		PackageHead * head = (PackageHead*) buffer;
		if((head->sessionId = manage.getSessionId(tSocket)) > 0){
			encrypt.encrypt(buffer + PHS, size);
			head->length = size;
			int send = udtSend(manage.getuSocket(tSocket), buffer, size + PHS);
			if(size == 0)
        UDT::epoll_remove_ssock(eid, tSocket);
			return send;
		}
	}
	closeTCP(eid, tSocket, 2, manage);
	return 0;
}

int downloadU2T(int eid, int uSocket, char* buffer, SessionManage &manage, Encrypt &encrypt)
{
	PackageHead * head = (PackageHead*) buffer;
	if(udtRecv(uSocket, buffer, PHS) == PHS){
		int tSocket;
		if((tSocket = manage.gettSocket(0,head->sessionId)) > 0){
			if(head->length == 0||head->length > BS-PHS){
				closeTCP(eid, tSocket, 2, manage);
			}
			if(udtRecv(uSocket, buffer + PHS, head->length)){
				int size = head->length;
				encrypt.decrypt(buffer + PHS, size);
				return tcpSend(tSocket, buffer + PHS, size);
			}
		}
		return -1;
	}
}
