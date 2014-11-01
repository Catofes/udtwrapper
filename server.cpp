#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <udt/udt.h>
#include <netdb.h>
#include <iostream>
#include <ctime>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>  
#include <errno.h>
#include <map>
using namespace std;

static string remoteAddress = "127.0.0.1";
static uint16_t remotePort = 10000;
static uint16_t localPort = 5555;
static int localSock;
static int udtSock;
static int bufferSize = 10240;
static int maxPending = 200;

struct PackageHead
{
	int sessionId;
	int length;
} __attribute__((packed));

struct ClientInfo
{
	int usocket;
	int sessionId;
};

bool operator < (const ClientInfo &l, const ClientInfo &r) {
	if(l.usocket == r.usocket)
	  return l.sessionId < r.sessionId;
	return l.usocket < r.usocket;
}

class SessionManage
{
	private:
		map<ClientInfo,int> clientinfo_tsocket;
		map<int,ClientInfo> tsocket_clientinfo;
		int randomSessionId()
		{
			return rand();
		}
	public:
		SessionManage(){};
		int add(int usocket, int sessionId, int tsocket)
		{
			ClientInfo data;
			data.sessionId = sessionId;
			data.usocket = usocket;
			clientinfo_tsocket[data]=tsocket;
			tsocket_clientinfo[tsocket]=data;
		}
		int remove(int ssocket)
		{
			return 1;
		}
		int gettSocket(int usocket, int sessionId)
		{
			ClientInfo data;
			data.sessionId = sessionId;
			data.usocket = usocket;
			if(clientinfo_tsocket.find(data) != clientinfo_tsocket.end())
			  return clientinfo_tsocket[data];
			return -1;
		}
		int getSessionId(int tsocket)
		{
			if(tsocket_clientinfo.find(tsocket) != tsocket_clientinfo.end())
			  return tsocket_clientinfo[tsocket].sessionId;
			return -1;
		}
		int getuSocket(int tsocket)
		{
			if(tsocket_clientinfo.find(tsocket) != tsocket_clientinfo.end())
			  return tsocket_clientinfo[tsocket].usocket;
			return -1;
		}
};

int tcp_create_socket()
{
	int sock;
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		cout << "[E] Could not create Socket." << endl;
		return -1; 
	}   
	sockaddr_in remote;
	if(inet_aton(remoteAddress.c_str(), &(remote.sin_addr)) == 0) {
		cout << "[E] Wrong Ip Address." << endl;
		return -2; 
	}   
	remote.sin_family = AF_INET;
	remote.sin_port = htons(remotePort);
	memset(&(remote.sin_zero), '\0', 8); 
	if(UDT::ERROR == connect(sock, (sockaddr*) &(remote), sizeof(remote))) {
		cout << "[E] connection failed." << endl;
		cout << "[D] " << UDT::getlasterror().getErrorMessage() << endl;
		return -3; 
	}   
	cout << "[I] connected to remote." <<endl;
	return sock;
}

int udt_create_socket()
{
	int sock;
	sockaddr_in local;
	if((sock = UDT::socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
		cout << "[E] Could not create UDT Socket." << endl;
		return -1; 
	}   
	local.sin_family = AF_INET;
	local.sin_port = htons(localPort);
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
	cout << "[I] UDT socket setup ok." << endl; 
	return sock;
}

void start_new_dail(int eid, SessionManage &manage)
{
	socklen_t clilen;
	int clilensize=sizeof(clilen);
	struct sockaddr_in clientaddr;
	int connfd = UDT::accept(udtSock, (sockaddr *)&clientaddr, &clilensize);
	if(connfd < 0) {
		perror("connfd < 0");
		exit(1);
	}
	char *str = inet_ntoa(clientaddr.sin_addr); 
	cout << "Accept a UDT connection from " << str<<":"<<ntohs(clientaddr.sin_port)<<endl;
	if(UDT::epoll_add_usock(eid, connfd) == UDT::ERROR) {
		cout << UDT::getlasterror().getErrorMessage() << endl;
		exit(1);
	}
}

int udt_recv_sock(int sock, char *buffer, uint32_t size)
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

int udt_send_sock(int sock, const char *buffer, uint32_t size)
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

int tcp_recv_sock(int sock, char *buffer, uint32_t size)
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

int tcp_send_sock(int sock, const char *buffer, uint32_t size)
{
	int index = 0, ret;
	while(size) {
		if((ret = send(sock, &buffer[index], size, 0)) <=0) {
			if(ret < 0)  
			  cout<<"tcp error" << strerror(errno);
			return (!ret) ? index : -1; 
		}
		index += ret;
		size -= ret;
	}   
	return index;
}

void handle_data_t2u(int eid, int socket, char* buffer, SessionManage &manage)
{
	int size = 0;
	size = recv(socket, &buffer[sizeof(PackageHead)] ,bufferSize - sizeof(PackageHead),0);
	if(size == 0){
		UDT::epoll_remove_ssock(eid, socket);
	}
	PackageHead * head = (PackageHead*) buffer;
	head->sessionId = manage.getSessionId(socket);
	head->length = size;
	int usocket = manage.getuSocket(socket);
	udt_send_sock(usocket, buffer, size + sizeof(PackageHead));
	if(size == 0){
		shutdown(socket,2);
		manage.remove(socket);
	}
}

void handle_data_u2t(int eid, UDTSOCKET socket, char* buffer, SessionManage &manage)
{
	PackageHead * head = (PackageHead*) buffer;
	if(udt_recv_sock(socket, buffer, sizeof(PackageHead))!=sizeof(PackageHead)){
		cout<<"Big Error. Reset"<<endl;
		UDT::epoll_remove_usock(eid,socket);
		UDT::close(socket);
		return;
	}
	int tsocket;
	if((tsocket = manage.gettSocket(socket, head->sessionId)) == -1){
		tsocket = tcp_create_socket();
		UDT::epoll_add_ssock(eid,tsocket);
		manage.add(socket, head->sessionId, tsocket);
	}
	if(head->length == 0){
		shutdown(tsocket, 2);
		return ;
	}
	if(head->length > bufferSize){
		cout<<"Read Error. Reset"<<endl;
		UDT::epoll_remove_usock(eid,socket);
		UDT::close(socket);
		return;
	}
	udt_recv_sock(socket, buffer + sizeof(PackageHead), head->length);
	tcp_send_sock(tsocket, buffer + sizeof(PackageHead), head->length);
}

int main()
{
	udtSock = udt_create_socket();
	cout<<"UDT Listen Sock:"<<udtSock<<endl;
	SessionManage sessionManage;
	char buffer[bufferSize];
	//create epoll
	int eid = UDT::epoll_create();
	if(UDT::epoll_add_usock(eid, udtSock) == UDT::ERROR) {
		cout << UDT::getlasterror().getErrorMessage() << endl;
		exit(1);
	}
	set<UDTSOCKET> readfds;
	set<int> sreadfds;
	//start loop
	while(true){
		int ret = UDT::epoll_wait(eid, &readfds, NULL, -1 , &sreadfds, NULL);
		//cout<<"Resume Num:"<<ret<<"	"<<endl;
		for(set<UDTSOCKET>::iterator i = readfds.begin(); i != readfds.end(); ++i){
			if(*i == udtSock){
				start_new_dail(eid, sessionManage);
			}else{
				handle_data_u2t(eid, *i, buffer, sessionManage);
			}
		}	
		for(set<int>::iterator i = sreadfds.begin(); i != sreadfds.end(); ++i){
			handle_data_t2u(eid, *i, buffer, sessionManage);
		}
	}
}
