#include <log.hh>
#include <error.hh>
#include "head.hh"

int Head::Read(char *data, uint16_t size)
{
    if (size <= 0) {
        throw Head::ParseZero();
    }
    type = (Head::HeadType) data[0];
    switch (type) {
        case CONNECT:
            ReadConnect(data, size);
            break;
        case DATA:
            ReadData(data, size);
            break;
        case FIN:
            ReadFin(data, size);
            break;
        case RST:
            ReadFin(data, size);
            break;
        default:
            Log::Log("Error Head Type.", 3);
            throw Head::UnknownType();
    }
    return 0;
}

int Head::ReadConnect(char *data, uint16_t size)
{
    if (size < 4)
        throw Head::Partial();
    address = (uint32_t) data[1];
    port = (uint16_t) data[3];
    return 0;
}

int Head::ReadData(char *data, uint16_t size)
{
    if (size < 2)
        throw Head::Partial();
    data_length = (uint16_t) data[1];
    return 0;
}