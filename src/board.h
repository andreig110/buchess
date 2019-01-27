#ifndef BOARD_H_INCLUDED
#define BOARD_H_INCLUDED

#include "types.h"

class Position;

SquareList pawn_attacks_from(const Position& pos, int file, int rank);
SquareList knight_attacks_from(int file, int rank);
SquareList rook_attacks_from(const Position& pos, int file, int rank);
SquareList bishop_attacks_from(const Position& pos, int file, int rank);
SquareList queen_attacks_from(const Position& pos, int file, int rank);
SquareList king_attacks_from(/*const Position& pos,*/ int file, int rank);

//template <PieceType Pt>
SquareList figure_attacks_from(const PieceType Pt, const Position& pos, int file, int rank);

VectorSquareList* figure_attacks_behind_king_from(const PieceType Pt, const Position& pos, int file, int rank, VectorSquareList* asbk);

#endif // #ifndef BOARD_H_INCLUDED
