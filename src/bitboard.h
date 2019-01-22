#ifndef BITBOARD_H_INCLUDED
#define BITBOARD_H_INCLUDED

#include "types.h"

namespace Bitboards {

void init();

}

constexpr Bitboard AllSquares = ~Bitboard(0);
constexpr Bitboard DarkSquares = 0xAA55AA55AA55AA55ULL;

constexpr Bitboard FileABB = 0x0101010101010101ULL;
constexpr Bitboard FileBBB = FileABB << 1;
constexpr Bitboard FileCBB = FileABB << 2;
constexpr Bitboard FileDBB = FileABB << 3;
constexpr Bitboard FileEBB = FileABB << 4;
constexpr Bitboard FileFBB = FileABB << 5;
constexpr Bitboard FileGBB = FileABB << 6;
constexpr Bitboard FileHBB = FileABB << 7;

constexpr Bitboard Rank1BB = 0xFF;
constexpr Bitboard Rank2BB = Rank1BB << (8 * 1);
constexpr Bitboard Rank3BB = Rank1BB << (8 * 2);
constexpr Bitboard Rank4BB = Rank1BB << (8 * 3);
constexpr Bitboard Rank5BB = Rank1BB << (8 * 4);
constexpr Bitboard Rank6BB = Rank1BB << (8 * 5);
constexpr Bitboard Rank7BB = Rank1BB << (8 * 6);
constexpr Bitboard Rank8BB = Rank1BB << (8 * 7);

extern Bitboard SquareBB[SQUARE_NB];
extern Bitboard FileBB[FILE_NB];
extern Bitboard RankBB[RANK_NB];


/// Overloads of bitwise operators between a Bitboard and a Square_int for testing
/// whether a given bit is set in a bitboard, and for setting and clearing bits.

inline Bitboard operator&(Bitboard b, Square_int s) {
  assert(s >= SQ_A1 && s <= SQ_H8);
  return b & SquareBB[s];
}

inline Bitboard operator|(Bitboard b, Square_int s) {
  assert(s >= SQ_A1 && s <= SQ_H8);
  return b | SquareBB[s];
}

inline Bitboard operator^(Bitboard b, Square_int s) {
  assert(s >= SQ_A1 && s <= SQ_H8);
  return b ^ SquareBB[s];
}

inline Bitboard& operator|=(Bitboard& b, Square_int s) {
  assert(s >= SQ_A1 && s <= SQ_H8);
  return b |= SquareBB[s];
}

inline Bitboard& operator^=(Bitboard& b, Square_int s) {
  assert(s >= SQ_A1 && s <= SQ_H8);
  return b ^= SquareBB[s];
}

#endif // #ifndef BITBOARD_H_INCLUDED
