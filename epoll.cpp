#define DEBUG
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

int tcpAcpt(int eid, int tSocket, SessionManage &manage)
{
	struct sockaddr_in clientaddr;
	socklen_t clilen =  sizeof(struct sockaddr_in);
#ifdef DEBUG
	cout << "[D] Start Accept TCP." << endl;
#endif
	int connfd = accept(tSocket, (sockaddr *)&clientaddr, &clilen);
	if(connfd < 0) {
		cout<<"[E] Can't accept TCP connection."<<endl;
		return -1;
	}
	char *str = inet_ntoa(clientaddr.sin_addr);
	cout << "[D] Accept a connection from " << str << ":" << ntohs(clientaddr.sin_port) << endl; 
	manage.generate(0, connfd);
	UDT::epoll_add_ssock(eid, connfd);
	return connfd;
}

int closeTCP(int eid, int tSocket, int num, SessionManage &manage)
{
	if(manage.getSessionId(tSocket) > 0){
		if(num = 0)
		  manage.rremove(tSocket);
		else if(num = 1)
		  manage.wremove(tSocket);
		else{
			manage.rremove(tSocket);
			manage.wremove(tSocket);
		}
	}
	UDT::epoll_remove_ssock(eid, tSocket);
#ifdef DEBUG
	cout<<"[D] Close TCP:"<<tSocket<<endl;
#endif
	shutdown(tSocket, num);
	return 0;
}

int closeUDT(int eid, int uSocket, SessionManage &mange)
{
#ifdef DEBUG
	cout<<"[D] Close UDT:"<<uSocket<<endl;
#endif
	UDT::epoll_remove_usock(eid, uSocket);
	UDT::close(uSocket);
	return 0;
}


int uploadT2U(int eid, int tSocket, int uSocket, char* buffer, SessionManage &manage, Encrypt &encrypt)
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

	//if TCP recv <= 0 . Mean TCP Close. Send disconnect package. And remove listen.
	if(size <= 0){
		UDT::epoll_remove_ssock(eid, tSocket);
		shutdown(tSocket,0);
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
		manage.rremove(tSocket);
	
	return send;
}

int uploadU2T(int eid, int uSocket, char* buffer, SessionManage &manage, string remoteAddress, int portNum, Encrypt &encrypt)
{
	PackageHead * head = (PackageHead*) buffer;
#ifdef DEBUG
	cout<<"[D] UploadU2T recv SIG UP."<<endl;
#endif
	//Check If this Sig is fake wakeup. If epoll works good, This should not use
	/*
	   UDTSET readfds;
	   timeval tv;
	   tv.tv_sec = 0;
	   tv.tv_usec = 1;
	   UD_SET(uSocket, &readfds);
	   int res = UDT::select(0, &readfds, NULL, NULL, &tv);
	   if (!((res != UDT::ERROR) && (UD_ISSET(u, &readfds))))
	   return ;//No Data Input.
	   */

	//Check If UDT have and error.
	/*
	if(UDT::getlasterror_code() != 0){
		cout<<"[E] UDT ERROR. At code:"<<UDT::getlasterror_code()<<endl;
		UDT::getlasterror().clear();
		closeUDT(eid, uSocket, manage);
		return 0;
	}
	*/

	//Read PackageHead.
	int receivebytes = udtRecv(uSocket, buffer, PHS);
	
	if(receivebytes <= 0){
		cout<<"[E] UDT ERROR. At code:"<<UDT::getlasterror_code()<<endl;
                UDT::getlasterror().clear();
                closeUDT(eid, uSocket, manage);
                return 0;
	}

	int tSocket;
	//If sessionId is new, Setup new tcp connection.
	if((tSocket = manage.gettSocket(uSocket, head->sessionId)) == -1){
		tSocket = tcpConnect(remoteAddress, portNum);
		setTimeOut(tSocket);
		UDT::epoll_add_ssock(eid, tSocket);
		manage.add(uSocket, head->sessionId, tSocket);
	}

	//If length = 0 . Means Stop Write on a TCP.
	if(head->length == 0||head->length > BS-PHS){
		shutdown(tSocket,1);
		manage.wremove(tSocket);
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
		UDT::epoll_remove_ssock(eid, tSocket);
		shutdown(tSocket, 0);
		size = 0;
	}
	head->length = size;

	//Encrypt Data.
	encrypt.encrypt(buffer + PHS, size);

	//Send Data.
#ifdef DEBUG
	cout<<"[D] DownloadT2U send. Size:"<<size<<endl;
#endif
	int send = udtSend(manage.getuSocket(tSocket), buffer, size + PHS);
	
	if(size == 0)
		manage.rremove(tSocket);
		
	return send;
}

int downloadU2T(int eid, int uSocket, char* buffer, SessionManage &manage, Encrypt &encrypt)
{
	PackageHead * head = (PackageHead*) buffer;
#ifdef DEBUG
	cout<<"[D] DownloadU2T recv SIG UP." <<endl;
#endif
	//Check If this Sig is fake wakeup. If epoll works good, This should not use
	/*
	   UDTSET readfds;
	   timeval tv;
	   tv.tv_sec = 0;
	   tv.tv_usec = 1;
	   UD_SET(uSocket, &readfds);
	   int res = UDT::select(0, &readfds, NULL, NULL, &tv);
	   if (!((res != UDT::ERROR) && (UD_ISSET(u, &readfds))))
	   return ;//No Data Input.
	 */

	//Check If UDT have and error.
	/*
	if(UDT::getlasterror_code() != 0){
		cout<<"[E] UDT ERROR. At code:"<<UDT::getlasterror_code()<<endl;
		UDT::getlasterror().clear();
		return 0;
	}
	*/

	//Read PackageHead.
	//Set UDT_RECVTIMEO
	int timeout = 1;
	UDT::setsockopt(uSocket, 0, UDT_RCVTIMEO, &timeout, sizeof(int));
	int receivebyte = udtRecv(uSocket, buffer, PHS);

	if(receivebyte < 0){
		cout<<"Error Sig Happend"<<endl;
		return 0;
	}
	timeout = -1;		
	UDT::setsockopt(uSocket, 0, UDT_RCVTIMEO, &timeout, sizeof(int));
	//No tSocket get. Client get a error Data. Remove the Data.
	int tSocket;
	if((tSocket = manage.gettSocket(0, head->sessionId)) < 0){
		udtRecv(uSocket, buffer + PHS, head->length);
		return 0;
	}

	//If length = 0 . Stop Write.
	if(head->length == 0||head->length > BS-PHS){
		shutdown(tSocket,1);
		manage.wremove(tSocket);
		return 0;
	}

	//Receive Data.	
	udtRecv(uSocket, buffer + PHS, head->length);

	//Decrypt Data.
	int size = head->length;
	encrypt.decrypt(buffer + PHS, size);
#ifdef DEBUG
	cout<<"[D] DownloadU2T send. Size:"<<size<<endl;
#endif

	//Send Data
	return tcpSend(tSocket, buffer + PHS, size);
}
