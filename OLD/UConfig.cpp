//
// Created by herbertqiao on 3/24/15.
//

#include <iostream>
#include <cstring>
#include "UConfig.h"

using namespace std;

UConfig::UConfig() {
    server_mode = true;
    ipv6 = false;
    tcp_max_pending = 1024;
    gc_fin_timeout = 60;
    gc_keepalive_timeout = 8000;
    gc_max_t = 100000;
    gc_max_s = 900;
    gc_p = 0.2;
}

bool UConfig::read_from_argv(int *argc, char **argv) {
    return true;
}

bool UConfig::read_from_file(string filename) {
    return true;
}