//
// Created by herbertqiao on 3/24/16.
//

#ifndef UDTWRAPPER_LOG_HH
#define UDTWRAPPER_LOG_HH

#include <iostream>
#include <time.h>

using std::cout;
using std::endl;
using std::string;

namespace Log
{

    void Log(const string &log, char level);

    void StdLog(const string &log, char level);
}
#endif //UDTWRAPPER_LOG_HH
