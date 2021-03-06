#include <cassert>

#include "board.h"
#include "position.h"

SquareList pawn_attacks_from(const Position& pos, int file, int rank)
{
    int up;
    switch (color_of(pos.piece_on(file, rank))) {
        case WHITE:
            up = 1;
            break;
        case BLACK:
            up = -1;
            break;
    }
    SquareList squareList;
    if (file > FILE_A)
        squareList.addSquare( Square(file - 1, rank + up) );
    if (file < FILE_H)
        squareList.addSquare( Square(file + 1, rank + up) );
    return squareList;
}

SquareList knight_attacks_from(int file, int rank)
{
    SquareList squareList;
    
    if (file > FILE_B) {
        if (rank > RANK_1)
            squareList.addSquare( Square(file - 2, rank - 1) );
        if (rank < RANK_8)
            squareList.addSquare( Square(file - 2, rank + 1) );
    }
    if (file > FILE_A) {
        if (rank > RANK_2)
            squareList.addSquare( Square(file - 1, rank - 2) );
        if (rank < RANK_7)
            squareList.addSquare( Square(file - 1, rank + 2) );
    }
    if (file < FILE_H) {
        if (rank > RANK_2)
            squareList.addSquare( Square(file + 1, rank - 2) );
        if (rank < RANK_7)
            squareList.addSquare( Square(file + 1, rank + 2) );
    }
    if (file < FILE_G) {
        if (rank > RANK_1)
            squareList.addSquare( Square(file + 2, rank - 1) );
        if (rank < RANK_8)
            squareList.addSquare( Square(file + 2, rank + 1) );
    }
    
    return squareList;
}

SquareList rook_attacks_from(const Position& pos, int file, int rank)
{
    SquareList squareList;
    
    for (int to_rank = rank + 1; to_rank <= RANK_8; ++to_rank) {
        squareList.addSquare( Square(file, to_rank) );
        if (pos.piece_on(file, to_rank) != NO_PIECE)
            break;
    }
    for (int to_rank = rank - 1; to_rank >= RANK_1; --to_rank) {
        squareList.addSquare( Square(file, to_rank) );
        if (pos.piece_on(file, to_rank) != NO_PIECE)
            break;
    }
    
    for (int to_file = file + 1; to_file <= FILE_H; ++to_file) {
        squareList.addSquare( Square(to_file, rank) );
        if (pos.piece_on(to_file, rank) != NO_PIECE)
            break;
    }
    for (int to_file = file - 1; to_file >= FILE_A; --to_file) {
        squareList.addSquare( Square(to_file, rank) );
        if (pos.piece_on(to_file, rank) != NO_PIECE)
            break;
    }
    
    return squareList;
}

SquareList bishop_attacks_from(const Position& pos, int file, int rank)
{
    SquareList squareList;
    
    for (int to_file = file + 1, to_rank = rank + 1; to_file <= FILE_H && to_rank <= RANK_8; ++to_file, ++to_rank) {
        squareList.addSquare( Square(to_file, to_rank) );
        if (pos.piece_on(to_file, to_rank) != NO_PIECE)
            break;
    }
    for (int to_file = file - 1, to_rank = rank - 1; to_file >= FILE_A && to_rank >= RANK_1; --to_file, --to_rank) {
        squareList.addSquare( Square(to_file, to_rank) );
        if (pos.piece_on(to_file, to_rank) != NO_PIECE)
            break;
    }
    
    for (int to_file = file + 1, to_rank = rank - 1; to_file <= FILE_H && to_rank >= RANK_1; ++to_file, --to_rank) {
        squareList.addSquare( Square(to_file, to_rank) );
        if (pos.piece_on(to_file, to_rank) != NO_PIECE)
            break;
    }
    for (int to_file = file - 1, to_rank = rank + 1; to_file >= FILE_A && to_rank <= RANK_8; --to_file, ++to_rank) {
        squareList.addSquare( Square(to_file, to_rank) );
        if (pos.piece_on(to_file, to_rank) != NO_PIECE)
            break;
    }

    return squareList;
}

SquareList queen_attacks_from(const Position& pos, int file, int rank)
{
    SquareList squareList = rook_attacks_from(pos, file, rank);
    SquareList tempList = bishop_attacks_from(pos, file, rank);
    squareList.push(tempList);
    return squareList;
}

SquareList king_attacks_from(int file, int rank)
{
    SquareList squareList;
    
    if (file > FILE_A) {
        if (rank > RANK_1)
            squareList.addSquare( Square(file - 1, rank - 1) );
        squareList.addSquare( Square(file - 1, rank) );
        if (rank < RANK_8)
            squareList.addSquare( Square(file - 1, rank + 1) );
    }
    if (rank > RANK_1)
        squareList.addSquare( Square(file, rank - 1) );
    if (rank < RANK_8)
        squareList.addSquare( Square(file, rank + 1) );
    if (file < FILE_H) {
        if (rank > RANK_1)
            squareList.addSquare( Square(file + 1, rank - 1) );
        squareList.addSquare( Square(file + 1, rank) );
        if (rank < RANK_8)
            squareList.addSquare( Square(file + 1, rank + 1) );
    }
    
    return squareList;
}


//template <PieceType Pt>
SquareList figure_attacks_from(const PieceType Pt, const Position& pos, int file, int rank)
{
    switch (Pt) {
    case PAWN:
        return pawn_attacks_from(pos, file, rank);
        break;
    case KNIGHT:
        return knight_attacks_from(file, rank);
        break;
    case BISHOP:
        return bishop_attacks_from(pos, file, rank);
        break;
    case ROOK:
        return rook_attacks_from(pos, file, rank);
        break;
    case QUEEN:
        return queen_attacks_from(pos, file, rank);
        break;
    case KING:
        return king_attacks_from(file, rank);
        break;
    default:
        break;
    }
}


namespace {
    
    void rook_attacks_behind_king_from(const Position& pos, int file, int rank, VectorSquareList* asbk)
    {
        const Piece ourKing = make_piece(pos.side_to_move(), KING);
        
        for (int to_rank = rank + 1; to_rank <= RANK_8; ++to_rank) {
            const Piece pc = pos.piece_on(file, to_rank);
            if (pc == ourKing  &&  ++to_rank <= RANK_8) {  // it does not check if the square (behind king) is occupied
                asbk->addSquare(Square(file, to_rank));
                return;  // don't return, only break if you want to generate legal moves in a game with multiple kings :)
            }
            if (pc != NO_PIECE)
                break;
        }
        for (int to_rank = rank - 1; to_rank >= RANK_1; --to_rank) {
            const Piece pc = pos.piece_on(file, to_rank);
            if (pc == ourKing  &&  --to_rank >= RANK_1) {
                asbk->addSquare(Square(file, to_rank));
                return;
            }
            if (pc != NO_PIECE)
                break;
        }
        
        for (int to_file = file + 1; to_file <= FILE_H; ++to_file) {
            const Piece pc = pos.piece_on(to_file, rank);
            if (pc == ourKing  &&  ++to_file <= FILE_H) {
                asbk->addSquare(Square(to_file, rank));
                return;
            }
            if (pc != NO_PIECE)
                break;
        }
        for (int to_file = file - 1; to_file >= FILE_A; --to_file) {
            const Piece pc = pos.piece_on(to_file, rank);
            if (pc == ourKing  &&  --to_file >= FILE_A) {
                asbk->addSquare(Square(to_file, rank));
                return;
            }
            if (pc != NO_PIECE)
                break;
        }
    }
    
    void bishop_attacks_behind_king_from(const Position& pos, int file, int rank, VectorSquareList* asbk)
    {
        const Piece ourKing = make_piece(pos.side_to_move(), KING);
        
        for (int to_file = file + 1, to_rank = rank + 1; to_file <= FILE_H && to_rank <= RANK_8; ++to_file, ++to_rank) {
            const Piece pc = pos.piece_on(to_file, to_rank);
            if (pc == ourKing  &&  ++to_file <= FILE_H  &&  ++to_rank <= RANK_8) {  // it does not check if the square (behind king) is occupied
                asbk->addSquare(Square(to_file, to_rank));
                return;
            }
            if (pc != NO_PIECE)
                break;
        }
        for (int to_file = file - 1, to_rank = rank - 1; to_file >= FILE_A && to_rank >= RANK_1; --to_file, --to_rank) {
            const Piece pc = pos.piece_on(to_file, to_rank);
            if (pc == ourKing  &&  --to_file >= FILE_A  &&  --to_rank >= RANK_1) {
                asbk->addSquare(Square(to_file, to_rank));
                return;
            }
            if (pc != NO_PIECE)
                break;
        }
        
        for (int to_file = file + 1, to_rank = rank - 1; to_file <= FILE_H && to_rank >= RANK_1; ++to_file, --to_rank) {
            const Piece pc = pos.piece_on(to_file, to_rank);
            if (pc == ourKing  &&  ++to_file <= FILE_H  &&  --to_rank >= RANK_1) {
                asbk->addSquare(Square(to_file, to_rank));
                return;
            }
            if (pc != NO_PIECE)
                break;
        }
        for (int to_file = file - 1, to_rank = rank + 1; to_file >= FILE_A && to_rank <= RANK_8; --to_file, ++to_rank) {
            const Piece pc = pos.piece_on(to_file, to_rank);
            if (pc == ourKing  &&  --to_file >= FILE_A  &&  ++to_rank <= RANK_8) {
                asbk->addSquare(Square(to_file, to_rank));
                return;
            }
            if (pc != NO_PIECE)
                break;
        }
    }
    
}  // namespace


// Return attacked squares behind king (by bishop, rook or queen)
VectorSquareList* figure_attacks_behind_king_from(const PieceType Pt, const Position& pos, int file, int rank, VectorSquareList* asbk)
{
    assert(Pt == BISHOP || Pt == ROOK || Pt == QUEEN);
    
    switch (Pt) {
    case BISHOP:
        bishop_attacks_behind_king_from(pos, file, rank, asbk);
        break;
    case ROOK:
        rook_attacks_behind_king_from(pos, file, rank, asbk);
        break;
    case QUEEN:
        bishop_attacks_behind_king_from(pos, file, rank, asbk);
        rook_attacks_behind_king_from(pos, file, rank, asbk);
        break;
    }
    
    return asbk;
}


/// between() returns a list representing all the squares between the two
/// given ones.

VectorSquareList between(const Square& s1, const Square& s2)
{
    VectorSquareList res;
    if (s1.file == s2.file)
        for (int r = std::min(s1.rank, s2.rank) + 1; r < std::max(s1.rank, s2.rank); ++r)
            res.addSquare(Square(s1.file, r));
    else if (s1.rank == s2.rank)
        for (int f = std::min(s1.file, s2.file) + 1; f < std::max(s1.file, s2.file); ++f)
            res.addSquare(Square(f, s1.rank));
    else if (abs(s2.file - s1.file) == abs(s2.rank - s1.rank)) {
        const int df = (s1.file < s2.file) ? +1 : -1;
        const int dr = (s1.rank < s2.rank) ? +1 : -1;
        for (   int f = s1.file + df, r = s1.rank + dr;
                f != s2.file && r != s2.rank;
                f += df, r += dr    )
            res.addSquare(Square(f, r));
    }
    return res;
}


/// aligned() returns true if the squares s1, s2 and s3 are aligned either on a
/// straight or on a diagonal line.

bool aligned(const Square& s1, const Square& s2, const Square& s3)
{
    const int df21 = s2.file - s1.file;
    const int dr21 = s2.rank - s1.rank;
    const int df31 = s3.file - s1.file;
    const int dr31 = s3.rank - s1.rank;
    
    //if ((!df21 && !df31) || (!dr21 && !dr31))
    if (!(df21 || df31) || !(dr21 || dr31))
        return true;
    else {
        bool res =  (df21 ==  dr21) && (df31 ==  dr31);
        if (!res)
            res  =  (df21 == -dr21) && (df31 == -dr31);
        return res;
    }
}
