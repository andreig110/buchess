#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

#include <string>

namespace LOG {
    void openFile();
    void closeFile();
    void log(int argc, char* argv[]);
    void log(std::string s);
}

#endif // #ifndef LOG_H_INCLUDED
