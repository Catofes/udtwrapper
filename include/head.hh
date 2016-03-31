//
// Created by herbertqiao on 3/24/16.
//

#ifndef UDTWRAPPER_HEAD_HH
#define UDTWRAPPER_HEAD_HH

#include <stdint.h>

const uint16_t BS = 2048;
const uint16_t DS = BS - 48;

namespace SessionSpace
{
    enum SessionStatus
    {
        INIT,
        CONNECTING,
        PIPE,
        CLOSE,
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
        INIT,
        CONNECTING,
        PIPE,
        WAITING,
        BLOCKING,
    };
}

namespace HeadSpace
{
    enum HeadType : uint8_t
    {
        CONNECT,
        DATA,
        FIN,
        RST,
        HeadTypeNum
    };
}

class Head
{
public:
    HeadSpace::HeadType type;

    // not necessarily the best way, just to tell you it works
    union {
        struct {
            uint32_t address;
            uint16_t port;
        };

        uint16_t data_length;
    };

    //uint8_t head_length;
    //bool flag = false;

    int Read(const char *data, uint16_t size);

    int Pack(char *data) const;

private:
    // { CONNECT, DATA, FIN, RST }
    static const int sizeOfType[HeadTypeNum] = { 7, 3, 0, 0 };

};

#endif //UDTWRAPPER_HEAD_HH
