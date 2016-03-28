#include <log.hh>
#include <error.hh>
#include "head.hh"
#include "string.h"

int Head::Read(char *data, uint16_t size)
{
    if (size <= 0) {
        throw HeadSpace::ParseZero();
    }
    type = (HeadSpace::HeadType) data[0];
    switch (type) {
        case HeadSpace::CONNECT:
            ReadConnect(data, size);
            break;
        case HeadSpace::DATA:
            ReadData(data, size);
            break;
        case HeadSpace::FIN:
            ReadFin(data, size);
            break;
        case HeadSpace::RST:
            ReadRST(data, size);
            break;
        default:
            Log::Log("Error Head Type.", 3);
            throw HeadSpace::UnknownType();
    }
    return 0;
}

int Head::ReadConnect(char *data, uint16_t size)
{
    if (size < 7)
        throw HeadSpace::Partial();
    memcpy(&address, data + 5, sizeof(address));
    memcpy(&port, data + 5, sizeof(port));
    return 0;
}

int Head::ReadData(char *data, uint16_t size)
{
    if (size < 3)
        throw HeadSpace::Partial();
    memcpy(&data_length, data + 1, sizeof(data_length));
    return 0;
}

int Head::WriteConnect(char *data)
{
    data[0] = type;
    memcpy(data + 1, &address, sizeof(address));
    memcpy(data + 5, &port, sizeof(port));
    return 7;
}

int Head::WriteData(char *data)
{
    data[0] = type;
    memcpy(data + 1, &data_length, sizeof(data_length));
    return 3;
}

int Head::Pack(char *data)
{
    int result = 0;
    switch (type) {
        case HeadSpace::CONNECT:
            result = WriteConnect(data);
            break;
        case HeadSpace::DATA:
            result = WriteData(data);
            break;
        case HeadSpace::FIN:
            result = WriteFin(data);
            break;
        case HeadSpace::RST:
            result = WriteRST(data);
            break;
    }
    return result;
}