//
// Created by herbertqiao on 3/24/16.
//

#include "log.hh"

void Log::Log(string input, char level)
{
    Log::StdLog(input, level);
}

void Log::StdLog(string input, char level)
{
//    if (level < 3)
//        return;
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    char buf[80];
    strftime(buf, 80, "%Y-%m-%d %H:%M:%S", local);
    cout << buf << " [" << (int) level << "] " << input << endl;
}
