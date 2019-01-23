#include "bitboard.h"

Bitboard SquareBB[SQUARE_NB];
Bitboard FileBB[FILE_NB];
Bitboard RankBB[RANK_NB];


/// Bitboards::init() initializes various bitboard tables. It is called at
/// startup and relies on global objects to be already zero-initialized.

void Bitboards::init() {
    
    for (Square_int s = SQ_A1; s <= SQ_H8; ++s)
        SquareBB[s] = (1ULL << s);
    
    for (File f = FILE_A; f <= FILE_H; ++f)
        FileBB[f] = f > FILE_A ? FileBB[f - 1] << 1 : FileABB;
    
    for (Rank r = RANK_1; r <= RANK_8; ++r)
        RankBB[r] = r > RANK_1 ? RankBB[r - 1] << 8 : Rank1BB;
    
}
