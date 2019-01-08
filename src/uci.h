#ifndef UCI_H_INCLUDED
#define UCI_H_INCLUDED

#include <string>

#include "types.h"

class Position;

namespace UCI {
    
    void loop();
    std::string square(Square s);
    std::string move(Move m/*, bool chess960*/);
    Move to_move(const Position& pos, std::string& str);
    
} // namespace UCI

#endif // #ifndef UCI_H_INCLUDED
