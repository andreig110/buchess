#include <sstream>

#include "misc.h"

using namespace std;

namespace {
    
    const string Version = "";
    
} // namespace

const string engine_info(bool to_uci)
{
    stringstream ss, date(__DATE__); // From compiler, format is "Sep 21 2008"
    
    ss << "Buchess " << Version << " " << date.str();
    ss << (to_uci  ? "\nid author ": " by ")
       << "Andrei Guga";
    
    return ss.str();
}
