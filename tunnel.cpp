#include <iostream>
#include <udt/udt.h>
#include <string.h>
#include <arpa/inet.h>
using namespace std;
#include "tunnel.h"

void setTimeOut(int tSocket)
{
	int nNetTimeout=60000;
	setsockopt(tSocket,SOL_SOCKET,SO_SNDTIMEO,(char *)&nNetTimeout,sizeof(int));
	setsockopt(tSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&nNetTimeout,sizeof(int));
}

int udtConnect(string remoteAddress, int portNum)
{
	cout << "[I] UDT Start Connect to " << remoteAddress << ":" << portNum << endl;
	int sock;
	if((sock = UDT::socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		cout << "[E] Could not create UDT Socket." << endl;
		return -1;
	}
	sockaddr_in remote;
	if(inet_aton(remoteAddress.c_str(), &(remote.sin_addr)) == 0) {
		cout << "[E] Wrong Ip Address." << endl;
		return -2;
	}
	remote.sin_family = AF_INET;
	remote.sin_port = htons(portNum);
	memset(&(remote.sin_zero), '\0', 8);
	if(UDT::ERROR == UDT::connect(sock, (sockaddr*) &(remote), sizeof(remote))) {
		cout << "[E] UDT connection failed." << endl;
		cout << "[D] " << UDT::getlasterror().getErrorMessage() << endl;
		return -3;
	}
	cout << "[I] UDT connect successfully." << endl;
	return sock;
}

int udtListen(int portNum, int maxPending)
{
	cout << "[I] UDT start to listen localport: " << portNum << endl;
	int sock;
	sockaddr_in local;
	if((sock = UDT::socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
		cout << "[E] Could not create UDT Socket." << endl;
		return -1;
	}
	local.sin_family = AF_INET;
	local.sin_port = htons(portNum);
	local.sin_addr.s_addr = INADDR_ANY;
	memset(&(local.sin_zero), '\0', 8);
	if(UDT::ERROR == UDT::bind(sock, (sockaddr*) &local, sizeof(local))) {
		cout << "[E] UDT bind failed." << endl;
		cout << "[D] " << UDT::getlasterror().getErrorMessage() << endl;
		return -2;
	}
	if (UDT::ERROR == UDT::listen(sock, maxPending)) {
		cout << "[E] Listen error." << endl;
		return -3;
	}
	cout << "[I] UDT listen sucessfully." << endl;
	return sock;
}

int tcpConnect(string remoteAddress, int portNum)
{
	int sock;
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		cout << "[E] Could not create Socket. Code at:"<<sock << endl;
		return -1;
	}
	sockaddr_in remote;
	if(inet_aton(remoteAddress.c_str(), &(remote.sin_addr)) == 0) {
		cout << "[E] Wrong Ip Address." << endl;
		return -2;
	}
	remote.sin_family = AF_INET;
	remote.sin_port = htons(portNum);
	memset(&(remote.sin_zero), '\0', 8);
	if(UDT::ERROR == connect(sock, (sockaddr*) &(remote), sizeof(remote))) {
		cout << "[E] connection failed." << endl;
		cout << "[D] " << UDT::getlasterror().getErrorMessage() << endl;
		return -3;
	}
	return sock;
}

int tcpListen(int portNum, int maxPending)
{
	int sock;
	sockaddr_in local;
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		cout << "[E] Could not create socket. Code at:"<<sock << endl;
		return -1;
	}
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(portNum);
	if (bind(sock, (struct sockaddr *) &local, sizeof(local)) < 0) {
		cout << "[E] Bind error." << endl;
		return -2;
	}
	if (listen(sock, maxPending) < 0) {
		cout << "[E] Listen error." << endl;
		return -3;
	}
	cout << "[I] Local socks5 server start." << endl;
	return sock;
}

int udtRecv(int sock, char *buffer, int size)
{
	int index = 0, ret;
	while(size) {
		if((ret = UDT::recv(sock, &buffer[index], size, 0)) <= 0)
		  return (!ret) ? index : -1;
		index += ret;
		size -= ret;
	}
	return index;

}

int udtRecvNoBlock(int sock, char *buffer, int size)
{
	bool blocking = false;
	UDT::setsockopt(sock, 0, UDT_RCVSYN, &blocking, sizeof(bool));
	UDT::getlasterror().clear();
	int index=0;
	int ret=UDT::recv(sock, &buffer[index], size, 0);
	blocking = true;
	UDT::setsockopt(sock, 0, UDT_RCVSYN, &blocking, sizeof(bool));
	if(ret <= 0){
		if(UDT::getlasterror_code() != 6002)
		  return -1;
		else{
		  cout<<"Sig Error."<<endl;
		  return -2;
		}
	}
	size -= ret;
	index += ret;
	while(size > 0) {
		if((ret = UDT::recv(sock, &buffer[index], size, 0)) <= 0)
		  return (!ret) ? index : -1;
		index += ret;
		size -= ret;
	}
	return index;
}

int udtSend(int sock, const char *buffer, int size)
{
	int index = 0, ret;
	while(size) {
		if((ret = UDT::send(sock, &buffer[index], size, 0)) <=0)
		  return (!ret) ? index : -1;
		index += ret;
		size -= ret;
	}
	return index;
}

int tcpRecv(int sock, char *buffer, int size)
{
	int index = 0, ret;
	while(size) {
		if((ret = recv(sock, &buffer[index], size, 0)) <= 0)
		  return (!ret) ? index : -1;
		index += ret;
		size -= ret;
	}
	return index;
}

int tcpSend(int sock, const char *buffer, int size)
{
	int index = 0, ret;
	while(size) {
		if((ret = send(sock, &buffer[index], size, 0)) <=0) {
			return (!ret) ? index : -1;
		}
		index += ret;
		size -= ret;
	}
	return index;
}
