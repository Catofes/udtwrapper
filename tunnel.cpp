/****************************
	Name:         UdtWrapper
	Author:       Catofes
	Date:         2014-11-1
	License:      All Right Reserved
	Description:  A tunnel to exchange tcp stream via udt.
*****************************/

#include <iostream>
#include <udt/udt.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
using namespace std;
#include "tunnel.h"
#include "config.h"

void setTimeOut(int tSocket)
{
	int nNetTimeout=60000;
	setsockopt(tSocket,SOL_SOCKET,SO_SNDTIMEO,(char *)&nNetTimeout,sizeof(int));
	setsockopt(tSocket,SOL_SOCKET,SO_RCVTIMEO,(char *)&nNetTimeout,sizeof(int));
}

int udtConnect(Config &config)
{
	cout << "[I] UDT Start Connect to " << config.remoteAddress << ":" << config.remotePort << endl;
	int sock;
	if(config.IPV6){
		if((sock = UDT::socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
			cout << "[E] Could not create UDT Socket." << endl;
			return -1;
		}
		sockaddr_in6 remote;
		memset(&remote, '\0', sizeof(remote));
		if(inet_pton(AF_INET6, config.remoteAddress.c_str(), &(remote.sin6_addr)) == 0) {
			cout << "[E] Wrong Ip Address." << endl;
			return -2;
		}
		remote.sin6_family = AF_INET6;
		remote.sin6_port = htons(config.remotePort);
		if(UDT::ERROR == UDT::connect(sock, (sockaddr*) &(remote), sizeof(remote))) {
			cout << "[E] UDT connection failed." << endl;
			cout << "[D] " << UDT::getlasterror().getErrorMessage() << endl;
			return -3;
		}
		cout << "[I] UDT connect successfully." << endl;
		return sock;

	}else{
	if((sock = UDT::socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		cout << "[E] Could not create UDT Socket." << endl;
		return -1;
	}
	sockaddr_in remote;
	if(inet_aton(config.remoteAddress.c_str(), &(remote.sin_addr)) == 0) {
		cout << "[E] Wrong Ip Address." << endl;
		return -2;
	}
	remote.sin_family = AF_INET;
	remote.sin_port = htons(config.remotePort);
	memset(&(remote.sin_zero), '\0', 8);
	if(UDT::ERROR == UDT::connect(sock, (sockaddr*) &(remote), sizeof(remote))) {
		cout << "[E] UDT connection failed." << endl;
		cout << "[D] " << UDT::getlasterror().getErrorMessage() << endl;
		return -3;
	}
	cout << "[I] UDT connect successfully." << endl;
	return sock;
	}
}

int udtListen(Config &config)
{
	cout << "[I] UDT start to listen localport: " << config.listenPort << endl;
	int sock;
	if(config.IPV6){
		sockaddr_in6 local;
		memset(&(local), '\0', sizeof(local));
		if((sock = UDT::socket(AF_INET6, SOCK_STREAM, 0)) <= 0) {
			cout << "[E] Could not create UDT Socket." << endl;
			return -1;
		}
		local.sin6_family = AF_INET6;
		local.sin6_port = htons(config.listenPort);
		local.sin6_addr = in6addr_any;
		if(UDT::ERROR == UDT::bind(sock, (sockaddr*) &local, sizeof(local))) {
			cout << "[E] UDT bind failed." << endl;
			cout << "[D] " << UDT::getlasterror().getErrorMessage() << endl;
			return -2;
		}
		if (UDT::ERROR == UDT::listen(sock, config.maxPending)) {
			cout << "[E] Listen error." << endl;
			return -3;
		}
		cout << "[I] UDT listen sucessfully." << endl;
		return sock;

	}
	sockaddr_in local;
	if((sock = UDT::socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
		cout << "[E] Could not create UDT Socket." << endl;
		return -1;
	}
	local.sin_family = AF_INET;
	local.sin_port = htons(config.listenPort);
	local.sin_addr.s_addr = INADDR_ANY;
	memset(&(local.sin_zero), '\0', 8);
	if(UDT::ERROR == UDT::bind(sock, (sockaddr*) &local, sizeof(local))) {
		cout << "[E] UDT bind failed." << endl;
		cout << "[D] " << UDT::getlasterror().getErrorMessage() << endl;
		return -2;
	}
	if (UDT::ERROR == UDT::listen(sock, config.maxPending)) {
		cout << "[E] Listen error." << endl;
		return -3;
	}
	cout << "[I] UDT listen sucessfully." << endl;
	return sock;
}

int tcpConnect(Config &config)
{
	int sock;
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		cout << "[E] Could not create Socket. Code at:"<<errno << endl;
		return -1;
	}
	sockaddr_in remote;
	if(inet_aton(config.remoteAddress.c_str(), &(remote.sin_addr)) == 0) {
		cout << "[E] Wrong Ip Address." << endl;
		return -2;
	}
	remote.sin_family = AF_INET;
	remote.sin_port = htons(config.remotePort);
	memset(&(remote.sin_zero), '\0', 8);
	if(UDT::ERROR == connect(sock, (sockaddr*) &(remote), sizeof(remote))) {
		cout << "[E] connection failed." << endl;
		cout << "[D] " << UDT::getlasterror().getErrorMessage() << endl;
		return -3;
	}
	return sock;
}

int tcpListen(Config &config)
{
	int sock;
	sockaddr_in local;
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		cout << "[E] Could not create socket. Code at:"<<sock << endl;
		return -1;
	}
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(config.listenPort);
	if (bind(sock, (struct sockaddr *) &local, sizeof(local)) < 0) {
		cout << "[E] Bind error." << endl;
		return -2;
	}
	if (listen(sock, config.maxPending) < 0) {
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

int udtRecvNoBlock(int eid, int sock, char *buffer, int size)
{
	bool blocking = false;
	UDT::setsockopt(sock, 0, UDT_RCVSYN, &blocking, sizeof(bool));
	UDT::getlasterror().clear();
	int index=0;
	int ret=UDT::recv(sock, &buffer[index], size, 0);
	blocking = true;
	UDT::setsockopt(sock, 0, UDT_RCVSYN, &blocking, sizeof(bool));
	if(ret <= 0){
		if(UDT::getlasterror_code() != 6002){
			return -1;
		}
		else{
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

bool SetSocketBlockingEnabled(int fd, bool blocking)
{
	if (fd < 0) return false;
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0) return false;
	flags = blocking ? (flags&~O_NONBLOCK) : (flags|O_NONBLOCK);
	return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
}

int tcpSendNoBlock(int sock, const char *buffer, int size)
{
	int ret = send(sock, buffer, size, 0);
	if(ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK) )
	  return -2;
	return ret;
}

