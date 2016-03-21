//
// Created by herbertqiao on 3/24/15.
//

#ifndef _UDTWRAPPER_UWRAPPER_H_
#define _UDTWRAPPER_UWRAPPER_H_


class UWrapper {
public:
    int id;
    int eid;
    UWrapper(int eid);
    UWrapper(int id, int eid);
    ~UWrapper();

    virtual bool connect_to(int address, int port);
    virtual bool listen_at(int address, int port);
    virtual int send(int length, char * buffer);
    virtual int read(int length, char * buffer);
    virtual int read_once(int length, char * buffer);
    virtual int send_once(int length, char * buffer);
    virtual int send_no_block(int length, char *buffer);
    virtual int read_on_block(int length, char *buffer);
};

class TCPUWrapper : UWrapper{
    TCPUWrapper(int eid);
};

class UDPUWrapper : UWrapper{
    UDPUWrapper(int eid);
};

#endif //_UDTWRAPPER_UWRAPPER_H_
