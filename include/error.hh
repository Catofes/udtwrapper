//
// Created by herbertqiao on 3/24/16.
//

#ifndef UDTWRAPPER_ERROR_HH
#define UDTWRAPPER_ERROR_HH

#include <stdexcept>
#include <log.hh>

using std::exception;

namespace Head
{
    class ParseZero : exception
    {
    public:
        ParseZero()
        { Log::Log("Parse 0 Size Head. Skip.", 1); };
    };

    class Partial : exception
    {
    public:
        Partial()
        { Log::Log("Particle Head.", 0); };
    };

    class UnknownType : exception
    {
    public:
        UnknownType()
        { Log::Log("Unknow Type", 5); };
    };
}
#endif //UDTWRAPPER_ERROR_HH
