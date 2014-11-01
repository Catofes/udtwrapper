#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <udt/udt.h>
#include <netdb.h>
#include <iostream>
#include <map>
#include <ctime>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>  
#include <errno.h>

using namespace std;

static string remoteAddress = "106.185.36.221";
static uint16_t remotePort = 5555;
static uint16_t localPort = 5555;
static int udtSock;
static int bufferSize = 10240;
static int localSock ;
static int maxPending = 200;

int socketnum=0;

struct PackageHead
{
	int sessionId;
	int length;
} __attribute__((packed));

void signal_callback_handler(int signum){
}

class SessionManage
{
	private:
		map<int,int> socket_session;
		map<int,int> session_socket;
		int randomSessionId()
		{
			return rand();
		}
	public:
		SessionManage(){};
		int generateSessionId(int socket)
		{
			int sessionId = randomSessionId();
			while(session_socket.find(sessionId) != session_socket.end())
			  sessionId = randomSessionId();
			socket_session[socket]=sessionId;
			session_socket[sessionId]=socket;
			return sessionId;
		}
		int removeSessionId(int socket)
		{
			return 1;
		}
		int getSocket(int sessionId)
		{
			if(session_socket.find(sessionId) != session_socket.end())
			  return session_socket[sessionId];
			return -1;
		}
		int getSessionId(int socket)
		{
			if(socket_session.find(socket) != socket_session.end())
			  return socket_session[socket];
			return -1;
		}
};

int udt_create_socket()
{
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
	remote.sin_port = htons(remotePort);
	memset(&(remote.sin_zero), '\0', 8); 
	if(UDT::ERROR == UDT::connect(sock, (sockaddr*) &(remote), sizeof(remote))) {
		cout << "[E] UDT connection failed." << endl;
		cout << "[D] " << UDT::getlasterror().getErrorMessage() << endl;
		return -3; 
	}   
	cout << "[I] UDT connected to remote." <<endl;
	return sock;
}

int tcp_create_socket()
{
	int sock;
	sockaddr_in local;
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		cout << "[E] Could not create socket." << endl;
		return -1;
	}
	//Set Client address
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(localPort);
	//Bind
	if (bind(sock, (struct sockaddr *) &local, sizeof(local)) < 0) {
		cout << "[E] Bind error." << endl;
		return -2;
	}
	//Listen
	if (listen(sock, maxPending) < 0) {
		cout << "[E] Listen error." << endl;
		return -3;
	}
	cout << "[I] Local socks5 server start." << endl;
	return sock;
}

void start_new_dail(int eid, SessionManage &manage)
{
	socklen_t clilen;
	struct sockaddr_in clientaddr;
	int connfd = accept(localSock, (sockaddr *)&clientaddr, &clilen);
	cout<<"socket num:"<<socketnum++<<endl;
	if(connfd < 0) {
		cout<<"[D] Error Happen conffd < 0"<<endl;
		perror("connfd < 0");
		exit(1);
	}
	char *str = inet_ntoa(clientaddr.sin_addr); 
	cout << "Accept a connection from " << str<<":"<<ntohs(clientaddr.sin_port)<<endl;
	manage.generateSessionId(connfd);
	if(UDT::epoll_add_ssock(eid, connfd) == UDT::ERROR) {
		cout << "[D] UDT Error: "<<UDT::getlasterror().getErrorMessage() << endl;
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
	if(size <= 0){
		size = 0;
		cout<<"Tcp close or error"<<endl;
		manage.removeSessionId(socket);
		UDT::epoll_remove_ssock(eid, socket);
		cout<<"socket num:"<<socketnum--<<endl;
		shutdown(socket,2);
	}
	PackageHead * head = (PackageHead*) buffer;
	head->sessionId = manage.getSessionId(socket);
	head->length = size;
	udt_send_sock(udtSock, buffer, size + sizeof(PackageHead));
}

void handle_data_u2t(int eid,UDTSOCKET socket, char* buffer, SessionManage &manage)
{
	PackageHead * head = (PackageHead*) buffer;
	if(udt_recv_sock(socket, buffer, sizeof(PackageHead))!=sizeof(PackageHead)){
		cout<<"Big Error. Exit"<<endl;
		exit(1);
	}
	int tcpsock = manage.getSocket(head->sessionId);
	if(head->length == 0)
	{
		UDT::epoll_remove_ssock(eid, socket);
		cout<<"socket num:"<<socketnum--<<endl;
		shutdown(tcpsock,2);
		return;
	}
	udt_recv_sock(socket, buffer + sizeof(PackageHead), head->length);
	if(tcp_send_sock(tcpsock, buffer + sizeof(PackageHead), head->length)<0)
	{
		UDT::epoll_remove_ssock(eid, socket);
		shutdown(tcpsock,2);
	};
}

int main()
{
	signal(SIGPIPE, signal_callback_handler);
	udtSock = udt_create_socket();
	localSock = tcp_create_socket();
	cout<<"UDT Connect Sock:"<<udtSock<<endl;
	cout<<"TCP Listen Sock:"<<localSock<<endl;
	SessionManage sessionManage;
	char buffer[bufferSize];
	//create epoll
	int eid = UDT::epoll_create();
	if(UDT::epoll_add_ssock(eid, localSock) == UDT::ERROR) {
		cout << "[D] UDT Error: "<<UDT::getlasterror().getErrorMessage() << endl;
		exit(1);
	}
	if(UDT::epoll_add_usock(eid, udtSock) == UDT::ERROR) {
		cout << "[D] UDT Error: "<<UDT::getlasterror().getErrorMessage() << endl;
		exit(1);
	}
	set<UDTSOCKET> readfds;
	set<int> sreadfds;
	//start loop
	while(true){
		int ret = UDT::epoll_wait(eid, &readfds, NULL, -1 , &sreadfds, NULL);
		//cout<<"Resume Num:"<<ret<<"	"<<endl;
		for(set<UDTSOCKET>::iterator i = readfds.begin(); i != readfds.end(); ++i)
		  handle_data_u2t(eid, *i, buffer, sessionManage);
		for(set<int>::iterator i = sreadfds.begin(); i != sreadfds.end(); ++i){
			if(*i == localSock){
				start_new_dail(eid, sessionManage);
			}else{
				handle_data_t2u(eid, *i, buffer, sessionManage);
			}
		}
	}
}
