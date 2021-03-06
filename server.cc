//
// Created by herbertqiao on 3/25/16.
//

#include <iostream>
#include <getopt.h>
#include "epoll.hh"

using namespace std;

int main(int argc, char *argv[])
{
    string local_address = "";
    string remote_address = "";
    uint16_t local_port = 0;
    uint16_t remote_port = 0;
    extern char *optarg;
    while (true) {
        const int option = getopt(argc, argv, "a:b:c:d:e");
        if (option == -1) break;
        switch (option) {
            case 'a':
                local_address = optarg;
                break;
            case 'b':
                local_port = atoi(optarg);
                break;
            case 'c':
                remote_address = optarg;
                break;
            case 'd':
                remote_port = atoi(optarg);
                break;
            case 'e':
                break;
        }
    }

    if (argc < 5) {
        cout << " -a listen_address\n -b listen_port\n -c remote_address\n -d remote_port" << endl;
        exit(0);
    }
    string str = "Listen at ";
    str += local_address;
    str += ":";
    str += to_string(local_port);
    Log::Log(str, 5);

    str = "Connect to ";
    str += remote_address;
    str += ":";
    str += to_string(remote_port);
    Log::Log(str, 5);

    UEpoll epoll;
    epoll.InitServer(local_address, local_port);
    epoll.SetDestination(remote_address, remote_port);
    epoll.Loop();
}