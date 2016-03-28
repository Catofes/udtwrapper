//
// Created by herbertqiao on 3/24/16.
//

#ifndef UDTWRAPPER_ERROR_HH
#define UDTWRAPPER_ERROR_HH

#include <stdexcept>
#include <log.hh>
#include <errno.h>
#include <udt.h>

class UException : public std::exception
{
public:
    UException()
    { };
};

namespace HeadSpace
{
    class ParseZero : public UException
    {
    public:
        ParseZero()
        { Log::Log("Parse 0 Size Head. Skip.", 2); };
    };

    class Partial : public UException
    {
    public:
        Partial()
        { Log::Log("Particle Head.", 1); };
    };

    class UnknownType : public UException
    {
    public:
        UnknownType()
        { Log::Log("Unknow Type", 5); };
    };
}

namespace UConnect
{
    class WrongIpAddress : public UException
    {
    public:
        WrongIpAddress()
        { Log::Log("UDT Get Wrong Ip Address.", 5); }
    };

    class ConnectionFailed : public UException
    {
    public:
        ConnectionFailed()
        { Log::Log("UDT Connection Failed.", 5); }
    };

    class EAgain : public UException
    {
    public:
        EAgain()
        { Log::Log("UDT EAgain.", 1); }
    };

    class EError : public UException
    {
    public:
        EError()
        { Log::Log("UDT READ/WRITE ERROR. RST.", 5); }
    };

    class EFin : public UException
    {
    public:
        EFin()
        { Log::Log("UDT Fin Received.", 2); }
    };
}

namespace TConnect
{
    class WrongIpAddress : public UException
    {
    public:
        WrongIpAddress()
        { Log::Log("UDT Get Wrong Ip Address.", 5); }
    };

    class ConnectionFailed : public UException
    {
    public:
        ConnectionFailed()
        { Log::Log("TCP Connection Failed.", 5); }
    };

    class EAgain : public UException
    {
    public:
        EAgain()
        { Log::Log("TCP EAgain.", 1); }
    };

    class EError : public UException
    {
    public:
        EError()
        { Log::Log("TCP READ/WRITE ERROR. RST.", 5); }
    };

    class EConnectError : public UException
    {
    public:
        EConnectError()
        { Log::Log("TCP Connect ERROR. RST.", 5); }
    };

    class EFin : public UException
    {
    public:
        EFin()
        { Log::Log("TCP Fin Received.", 2); }
    };

    class GetSockOptError : public UException
    {
    public:
        GetSockOptError()
        { Log::Log("Get TCP Socket Option Error.", 3); }
    };
}

namespace SManager
{
    class NotFound : public UException
    {
    public:
        NotFound()
        { Log::Log("Session Not Found.", 3); }
    };
}

namespace EpollSpace
{
    class TcpAcceptError : public UException
    {
    public:
        TcpAcceptError()
        { Log::Log("Tcp Accept Error. " + errno, 3); }
    };

    class UdtAcceptError : public UException
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

namespace SessionSpace
{
    class ErrorStatus : public UException
    {
    public:
        ErrorStatus()
        { Log::Log("Error Session Status.", 3); }
    };

    class SendData2Fin : public UException
    {
    public:
        SendData2Fin()
        { Log::Log("Send Data To Fin Socket.", 3); }
    };
}
#endif //UDTWRAPPER_ERROR_HH
