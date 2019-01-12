#ifndef MOVEGEN_H_INCLUDED
#define MOVEGEN_H_INCLUDED

#include <algorithm>

#include "types.h"

class Position;

namespace MOVEGEN {
    bool check_move(const Position& pos, Move move);
}

enum GenType {
    CAPTURES,
    QUIETS,
    QUIET_CHECKS,
    EVASIONS,
    NON_EVASIONS,
    LEGAL
};

struct ExtMove {
    Move move;
    //int value;  TODO

    operator Move() const { return move; }
    void operator=(Move m) { move = m; }

    // Inhibit unwanted implicit conversions to Move
    // with an ambiguity that yields to a compile error.
    operator float() const = delete;
};

template<GenType>
ExtMove* generate(const Position& pos, ExtMove* moveList);

/// The MoveList struct is a simple wrapper around generate(). It sometimes comes
/// in handy to use this class instead of the low level generate() function.
template<GenType T>
struct MoveList {

    explicit MoveList(const Position& pos) : last(generate<T>(pos, moveList)) {}
    const ExtMove* begin() const { return moveList; }
    const ExtMove* end() const { return last; }
    size_t size() const { return last - moveList; }
    bool contains(Move move) const
    {
        return std::find(begin(), end(), move) != end();
    }

private:
    ExtMove moveList[MAX_MOVES], *last;
};

#endif // #ifndef MOVEGEN_H_INCLUDED
