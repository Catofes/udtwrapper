all:UDTSocks

UDTSocks:
	g++ client.cpp -o client.out -ludt -lpthread 
	g++ server.cpp -o server.out -ludt -lpthread 
debug:
	g++ -g client.cpp -o client.out -ludt -lpthread 
	g++ -g server.cpp -o server.out -ludt -lpthread 
clean:
	rm client.out
