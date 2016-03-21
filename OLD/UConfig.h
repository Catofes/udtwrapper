//
// Created by herbertqiao on 3/24/15.
//

#ifndef _UDTWRAPPER_UCONFIG_H_
#define _UDTWRAPPER_UCONFIG_H_

class UConfig {
public:

    //True for server mode. False for client mode.
    bool server_mode;
    //True for ipv6 mode. False for ipv4 mode.
    bool ipv6;

    int tcp_listen_port;
    string tcp_listen_address;
    int tcp_max_pending;
    int tcp_connect_port;
    string tcp_connect_address;
    int udt_listen_port;
    string udt_listen_address;
    int udt_connect_port;
    string udt_connect_address;

    /************************************
    * gc follow this model
    *
    *		if_gc		=	random(0,1) < (t / max_t)^2 && random(0,1) < (nowsockets/s)^2;
    *		gc_num		=	random(1, (sockets size) * p * (t / max_t) ^ 3
    *
    *		"t" is average time accross between two tsocket wakeup.
    *				t = 0.2*t + (time accross now)*0.8
    *
    *		So in each time tcp wakeup , caculate t and to choice if start garbage collection.
    *		Garbage collection will try to collection gc_num sockets.
    *
    ***********************************/
    int gc_fin_timeout;
    int gc_keepalive_timeout;
    float gc_max_t;
    float gc_max_s;
    float gc_p;

    int buffer_overflow;

    UConfig();
    ~UConfig();

    bool read_from_argv(int *argc, char **argv);
    bool read_from_file(string filename);
};

#endif //_UDTWRAPPER_UCONFIG_H_
