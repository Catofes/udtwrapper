//
// Created by herbertqiao on 3/24/16.
//

#ifndef UDTWRAPPER_ERROR_HH
#define UDTWRAPPER_ERROR_HH

#include <stdexcept>
#include <log.hh>
#include <errno.h>
#include <udt.h>

class UException : std::exception
{
public:
    UException()
    { };
};

namespace HeadSpace
{
    class ParseZero : UException
    {
    public:
        ParseZero()
        { Log::Log("Parse 0 Size Head. Skip.", 1); };
    };

    class Partial : UException
    {
    public:
        Partial()
        { Log::Log("Particle Head.", 0); };
    };

    class UnknownType : UException
    {
    public:
        UnknownType()
        { Log::Log("Unknow Type", 5); };
    };
}

namespace UConnect
{
    class WrongIpAddress : UException
    {
    public:
        WrongIpAddress()
        { Log::Log("UDT Get Wrong Ip Address.", 5); }
    };

    class ConnectionFailed : UException
    {
    public:
        ConnectionFailed()
        { Log::Log("UDT Connection Failed.", 5); }
    };
}

namespace TConnect
{
    class WrongIpAddress : UException
    {
    public:
        WrongIpAddress()
        { Log::Log("UDT Get Wrong Ip Address.", 5); }
    };

    class ConnectionFailed : UException
    {
    public:
        ConnectionFailed()
        { Log::Log("UDT Connection Failed.", 5); }
    };
}

namespace SManager
{
    class NotFound : UException
    {
    public:
        NotFound()
        { Log::Log("Session Not Found.", 3); }
    };
}

namespace EpollSpace
{
    class TcpAcceptError : UException
    {
    public:
        TcpAcceptError()
        { Log::Log("Tcp Accept Error. " + errno, 3); }
    };

    class UdtAcceptError : UException
    {
    public:
        UdtAcceptError()
        {
            string str = "Udt Accept Error. ";
            str += UDT::getlasterror().getErrorMessage();
            Log::Log(str, 3);
        }
    };
}
#endif //UDTWRAPPER_ERROR_HH
