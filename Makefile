all:UDTSocks

UDTSocks:
	g++ -c SessionManage.cpp
	g++ -c tunnel.cpp 
	g++ -c epoll.cpp
	g++ -c encrypt.cpp
	g++ -c server.cpp
	g++ -c client.cpp
	g++ SessionManage.o tunnel.o epoll.o encrypt.o server.o -o server.out -ludt -lpthread
	g++ SessionManage.o tunnel.o epoll.o encrypt.o client.o -o client.out -ludt -lpthread
	rm *.o
debug:
	g++ -g -c SessionManage.cpp
	g++ -g -c tunnel.cpp 
	g++ -g -c epoll.cpp
	g++ -g -c encrypt.cpp
	g++ -g -c server.cpp
	g++ -g -c client.cpp
	g++ SessionManage.o tunnel.o epoll.o encrypt.o server.o -o server.out -ludt -lpthread
	g++ SessionManage.o tunnel.o epoll.o encrypt.o client.o -o client.out -ludt -lpthread
	rm *.o

clean:
