//
// Created by herbertqiao on 3/24/16.
//

#ifndef UDTWRAPPER_HEAD_HH
#define UDTWRAPPER_HEAD_HH

#include <stdint.h>

const int BS=2048;

namespace SessionSpace
{
    enum SessionStatus
    {
        INIT,
        CONNECTING,
        PIPE,
        UPLOADING,
        DOWNLOADING,
        FIN,
    };

    enum Direction
    {
        TCP2UDT,
        UDT2TCP,
    };
}

namespace Connection
{
    enum ConnectionStatus
    {
        CONNECTING,
        READING,
        WRITING,
        FIN,
        CLOSED,
    };
}

namespace HeadSpace
{
    enum HeadType
    {
        CONNECT,
        DATA,
        FIN,
        RST,
    };
}

class Head
{
public:
    HeadSpace::HeadType type;
    uint8_t head_length;
    uint32_t address;
    uint16_t port;
    uint16_t data_length;
    bool flag;

    Head()
    {
        flag = false;
    }

    int Read(char *data, uint16_t size);

private:
    int ReadConnect(char *data, uint16_t size);

    int ReadData(char *data, uint16_t size);

    inline int ReadFin(char *data, uint16_t size)
    { return 0; }

    inline int ReadRST(char *data, uint16_t size)
    { return 0; }

};

#endif //UDTWRAPPER_HEAD_HH
