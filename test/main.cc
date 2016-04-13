//
// Created by herbertqiao on 4/13/16.
//

#include "main.hh"
#include <iostream>
#include <udt.h>
#include <arpa/inet.h>
#include <string.h>

using namespace std;

int main()
{
    UDT::startup();
    for (int i = 0; i < 10; i++) {
        UDTSOCKET client = UDT::socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(5000);
        inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

        memset(&(serv_addr.sin_zero), '\0', 8);

// connect to the server, implict bind
        if (UDT::ERROR == UDT::connect(client, (sockaddr *) &serv_addr, sizeof(serv_addr))) {
            cout << "connect: " << UDT::getlasterror().getErrorMessage();
            return 0;
        }
        UDT::close(client);
    }

    getchar();
}