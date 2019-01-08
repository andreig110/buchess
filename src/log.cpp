#include <fstream>
#include <ctime>
#include <string>

#include "log.h"

namespace {
    const std::string filename = "bce.log";
    std::ofstream outfile;
}

void LOG::openFile() {
    outfile.open(filename, std::ios::app);
    outfile << "\n\n\n";
}

void LOG::closeFile() {
    outfile.close();
}

void LOG::log(int argc, char* argv[])
{
    std::time_t t = std::time(0);   // get time now
    std::tm* now = std::localtime(&t);
    outfile << (now->tm_year + 1900) << '-' 
            << (now->tm_mon + 1) << '-'
            <<  now->tm_mday << ' '
            <<  now->tm_hour << ':'
            <<  now->tm_min << ':'
            <<  now->tm_sec
            << std::endl;
    
    outfile << "You have entered " << argc << " arguments:" << std::endl;
    for (int i = 0; i < argc; ++i)
        outfile << argv[i] << std::endl;
}

void LOG::log(std::string s) {
    outfile << s << std::endl;
}
